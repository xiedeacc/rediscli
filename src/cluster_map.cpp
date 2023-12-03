#include "cluster_map.h"

#include <algorithm>
#include <stdexcept>

#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "command/cluster.h"
#include "redis.h"
#include "src/detail/redis_channel_cache.h"
#include "src/nlog.h"

unsigned int keyHashSlot(char *key, int keylen);

namespace {
const redis::CLUSTER<redis::SLOTS> clusterslots;
}  // namespace
namespace redis {

Server::Server() { this->Clear(); }

void Server::Clear() {
  ip.clear();
  port = -1;
}

bool Server::IsValid() const { return !ip.empty() && port > 0; }

std::string Server::ToString() const { return fmt::format("{}:{}", ip, port); }

bool Server::Parse(const std::string &addr) {
  std::vector<std::string> tokens;
  boost::split(tokens, addr, [](auto c) { return c == ':'; });
  if (tokens.size() != 2) {
    LOG_CRITICAL("invalid redis server: {}", addr);
    return false;
  }
  boost::trim(tokens[0]);
  boost::trim(tokens[1]);
  if (tokens[0].empty() || tokens[1].empty()) {
    LOG_CRITICAL("invalid redis addr: {}", addr);
    return false;
  }
  this->ip = std::move(tokens[0]);
  this->port = atoi(tokens[1].c_str());
  return true;
}

Slot::Slot() { this->Clear(); }
Slot::~Slot() { this->Clear(); }

void Slot::Clear() {
  beg_ = 0;
  end_ = -1;
  master_.Clear();
  slaves_.clear();
}

bool Slot::IsValid() const { return end_ >= beg_ && master_.IsValid(); }
int Slot::Begin() const { return beg_; }
int Slot::End() const { return end_; }
void Slot::Begin(int beg) { beg_ = beg; }
void Slot::End(int end) { end_ = end; }

const Server &Slot::Master() const { return master_; }
void Slot::Master(const std::string &ip, int port) {
  master_.ip = ip;
  master_.port = port;
}
void Slot::Master(const Server &server) { master_ = server; }
void Slot::Master(Server &&server) { master_ = std::move(server); }

int Slot::SlaveNum() const { return slaves_.size(); }
const Server &Slot::Slave(int i) const {
  static Server ss;
  if (i >= SlaveNum()) {
    return ss;
  }
  return slaves_[i];
}
void Slot::AddSlave(const std::string &ip, int port) {
  Server s;
  s.ip = ip;
  s.port = port;
  slaves_.emplace_back(std::move(s));
}
void Slot::AddSlave(const Server &server) { slaves_.push_back(server); }
void Slot::AddSlave(Server &&server) {
  slaves_.emplace_back(std::move(server));
}

void Slot::Print() const {
  // LOG_DEBUG("slot range[{}, {}]", beg_, end_);
  // LOG_DEBUG("     master: {}", master_.ToString());
  for (size_t i = 0; i < slaves_.size(); ++i) {
    // LOG_DEBUG("     slave[{}]: {}", i, slaves_[i].ToString());
  }
}

int SlotMap::GetSlot(const std::string &key) const {
  return ::keyHashSlot((char *)key.c_str(), key.length());
}

void SlotMap::Print() const {
  std::for_each(slot_map_.begin(), slot_map_.end(),
                [](auto &p) { p.second.Print(); });
}

int SlotMap::InitChannel() {
  auto it = Begin();
  auto end = End();
  for (; it != end; it++) {
    const auto &slot = it->second;
    if (!detail::RedisChannelCache::Instance().RegisterChannel(
            GetClusterName(), slot.Master().ip, slot.Master().port)) {
      return -1;
    }
    for (int i = 0; i < slot.SlaveNum(); i++) {
      if (!detail::RedisChannelCache::Instance().RegisterChannel(
              GetClusterName(), slot.Slave(i).ip, slot.Slave(i).port)) {
        return -1;
      }
    }
  }
  return 0;
}

int SlotMap::GetSlotFromServer(const Server &svr) {
  try {
    Redis executor(GetClusterName(), svr.ip, svr.port, 500);
    auto reply = executor.Execute(clusterslots);
    auto &redis_reply = reply.GetRedisReply();

    if (!redis_reply.is_array() || redis_reply.size() == 0) {
      return -1;
    }

    for (size_t i = 0; i < redis_reply.size(); ++i) {
      auto &slot_info = redis_reply[i];
      Slot slot;

      slot.Begin(slot_info[0].integer());
      slot.End(slot_info[1].integer());

      Server master;
      master.ip = slot_info[2][0].c_str();
      master.port = slot_info[2][1].integer();
      slot.Master(std::move(master));

      for (int j = 3; j < slot_info.size(); ++j) {
        Server slave;
        slave.ip = slot_info[j][0].c_str();
        slave.port = slot_info[j][1].integer();
        slot.AddSlave(std::move(slave));
      }

      Add(slot);
    }
    return 0;
  } catch (std::exception &e) {
    LOG_CRITICAL("failed to get cluster slots from {}:{}: {}", svr.ip.c_str(),
                 svr.port, e.what());
    return -1;
  }
}

int SlotMap::Load(const std::string &content) {
  std::string str(content);

  boost::trim(str);
  std::vector<std::string> lines;
  boost::split(lines, str, [](auto c) { return c == '\n'; });

  if (lines.size() < 2) {
    LOG_CRITICAL("wrong format of redis config: {}", content);
    return -1;
  }

  for (auto &line : lines) {
    boost::trim(line);
  }

  SetClusterName(lines[0]);

  std::vector<Server> all_servers;
  all_servers.reserve(lines.size() - 1);
  std::for_each(lines.begin() + 1, lines.end(), [&all_servers](auto &l) {
    Server s;
    if (s.Parse(l) && s.IsValid()) {
      all_servers.emplace_back(std::move(s));
    } else {
      LOG_CRITICAL("failed to parse redis config line: {}", l);
    }
  });

  std::vector<int> pos;
  pos.reserve(all_servers.size());
  for (int i = 0; i < all_servers.size(); ++i) {
    pos.push_back(i);
  }
  std::random_shuffle(pos.begin(), pos.end()); /* 打乱顺序 */

  for (auto idx : pos) {
    const auto &svr = all_servers[idx];

    if (0 == GetSlotFromServer(svr)) {
      Freeze();
      Print();
      return InitChannel();
    }
  }

  LOG_CRITICAL("failed to get redis cluster slots from all instance");
  return -1;
}

SlotMap::SlotMap() { this->Freeze(); }
SlotMap::~SlotMap() {}

void SlotMap::Clear() {
  slot_map_.clear();
  this->Freeze();
}

bool SlotMap::Add(const Slot &slot) {
  if (!slot.IsValid()) {
    return false;
  }
  slot_map_.emplace(slot.Begin(), slot);
  return true;
}
void SlotMap::Freeze() { beg_ = slot_map_.begin(); }

const Slot *SlotMap::Find(int slot) const {
  auto it = slot_map_.upper_bound(slot);
  if (it != slot_map_.begin()) {
    --it;
    if (slot <= it->second.End()) {
      return &it->second;
    }
  }
  return nullptr;
}

Server SlotMap::FindWrite(int slot) const {
  const Slot *sl = this->Find(slot);
  if (sl) {
    return sl->Master();
  }
  Server dummy;
  return dummy;
}

Server SlotMap::FindRead(int slot, int master_ratio) const {
  const int max = 10000;
  if (master_ratio > max) {
    master_ratio = max;
  } else if (master_ratio < 0) {
    master_ratio = 0;
  }
  const Slot *sl = this->Find(slot);
  if (sl == nullptr) {
    Server dummy;
    return dummy;
  }
  int sn = sl->SlaveNum();
  if (sn > 0) {
    const long total = long(sn + 1) * max;
    if (double(total) * ::rand() / RAND_MAX < master_ratio) {
      return sl->Master(); /* 选中从master读 */
    }
    return sl->Slave(::rand() % sn);
  } else { /* 没有slave，返回master */
    return sl->Master();
  }
}

void SlotMap::SetClusterName(const std::string &name) { cluster_name_ = name; }

const std::string &SlotMap::GetClusterName() const { return cluster_name_; }

std::map<int, Slot>::const_iterator SlotMap::Begin() const {
  return slot_map_.cbegin();
}
std::map<int, Slot>::const_iterator SlotMap::End() const {
  return slot_map_.cend();
}
}  // namespace redis
