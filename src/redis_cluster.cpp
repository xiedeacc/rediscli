#include "redis_cluster.h"

#include <brpc/controller.h>
#include <brpc/parallel_channel.h>
#include <brpc/redis.h>
#include <fmt/format.h>

#include <boost/functional/hash.hpp>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "cluster_map.h"
#include "command/readonly.h"
#include "detail/async_data.h"
#include "detail/redis_channel_cache.h"
#include "redis.h"
using namespace std;
namespace pb = google::protobuf;

namespace {
const redis::READONLY readonly;

using Mapper = unordered_map<pair<string, int>, vector<int>, boost::hash<pair<string, int>>>;

struct ClusterArguments {
  redis::SlotMap *slotmap{nullptr};
  bool use_master{false};
  int timeout{0};
  const string *passwd;
};

class RedisClusterCallMapper : public brpc::CallMapper {
 public:
  RedisClusterCallMapper(vector<brpc::RedisRequest> &&reqs, redis::Reply *reply)
      : _reqs{move(reqs)}, _reply{reply} {
    _chans.reserve(reqs.size());
  }

  virtual brpc::SubCall Map(int channel_index, const pb::MethodDescriptor *method,
                            const pb::Message *request, pb::Message *response) override {
    return brpc::SubCall(nullptr, &_reqs[channel_index], _reply->GetResponse(channel_index), 0);
  }

  void AddChannel(shared_ptr<brpc::Channel> chan) { _chans.emplace_back(move(chan)); }

 private:
  vector<brpc::RedisRequest> _reqs;
  redis::Reply *_reply{nullptr};
  vector<shared_ptr<brpc::Channel>> _chans;
};

class RedisClusterResponseMerger : public brpc::ResponseMerger {
 public:
  virtual Result Merge(pb::Message *response, const pb::Message *sub_response) override {
    return MERGED;
  }
};
brpc::RedisRequest dummy_req;
brpc::RedisResponse dummy_resp;

pair<string, int> GetServer(const redis::Command &cmd, const ClusterArguments &cluster) {
  auto slotmap = cluster.slotmap;
  auto slot = slotmap->GetSlot(cmd.Key());
  redis::Server svr;
  if (cluster.use_master || cmd.Type() == redis::CommandType::Write) {
    svr = slotmap->FindWrite(slot);
  } else {
    svr = slotmap->FindRead(slot);
  }
  return {svr.ip, svr.port};
}

Mapper BuildMapper(const vector<const redis::Command *> &commands,
                   const ClusterArguments &cluster) {
  Mapper mapping;
  for (size_t i = 0; i < commands.size(); ++i) {
    auto cmd = commands[i];
    auto svr = GetServer(*cmd, cluster);
    mapping[svr].push_back(i);
  }
  return mapping;
}

vector<brpc::RedisRequest> BuildRequestResponse(const Mapper &mapping,
                                                const vector<const redis::Command *> &commands,
                                                const ClusterArguments &cluster,
                                                redis::Reply *reply) {
  vector<brpc::RedisRequest> reqs;
  reqs.reserve(mapping.size());
  for (auto &[svr, indexes] : mapping) {
    brpc::RedisRequest req;
    int offset = 0;
    if (!cluster.use_master && commands[indexes[0]]->Type() == redis::CommandType::Read) {
      readonly.AddTo(&req);
      offset += 1;
    }
    for (auto i : indexes) {
      auto cmd = commands[i];
      cmd->AddTo(&req);
    }
    reqs.emplace_back(move(req));
    reply->AddResponse(indexes, offset);
  }
  return reqs;
}

void BuildPChan(brpc::ParallelChannel &pchan, const Mapper &mapping,
                vector<brpc::RedisRequest> &&reqs, redis::Reply *reply,
                const ClusterArguments &cluster) {
  auto call_mapper = make_unique<RedisClusterCallMapper>(move(reqs), reply);
  auto response_merger = make_unique<RedisClusterResponseMerger>();
  bool need_release = false;

  brpc::ParallelChannelOptions opts;
  opts.fail_limit = 1;

  if (0 != pchan.Init(&opts)) {
    throw runtime_error("failed to init pchan");
  }

  for (auto &[svr, indexes] : mapping) {
    auto chan = redis::detail::RedisChannelCache::Instance().GetChannel(
        cluster.slotmap->GetClusterName(), svr.first, svr.second, *cluster.passwd);
    if (chan == nullptr) {
      throw runtime_error(fmt::format("failed to get redis channel {}:{}", svr.first, svr.second));
    }
    if (0 != pchan.AddChannel(chan.get(), brpc::DOESNT_OWN_CHANNEL, call_mapper.get(),
                              response_merger.get())) {
      throw runtime_error(fmt::format("failed to add redis channel {}:{}", svr.first, svr.second));
    }
    call_mapper->AddChannel(chan);
    need_release = true;
  }

  if (need_release) {
    call_mapper.release();
    response_merger.release();
  }
}

void SetControllerAttr(brpc::Controller *cntl, const ClusterArguments &cluster) {
  if (cluster.timeout > 0) {
    cntl->set_timeout_ms(cluster.timeout);
  }
}

int BuildExecutor(redis::Redis &exec, const redis::Command **cmds, size_t size,
                  const ClusterArguments &cluster) {
  int offset = 0;
  if (cmds[0]->Type() == redis::CommandType::Read && !cluster.use_master) {
    exec.Add(readonly);
    offset += 1;
  }
  for (size_t i = 0; i < size; ++i) {
    exec.Add(*cmds[i]);
  }
  return offset;
}

}  // namespace

namespace redis {
RedisCluster::RedisCluster(const string &cluster_name, int timeout) {}

RedisCluster::RedisCluster(SlotMap *slotmap, int timeout, const string &passwd)
    : _slot_map{slotmap}, _timeout{timeout}, _passwd{passwd} {
  if (slotmap == nullptr) {
    throw invalid_argument(fmt::format("slotmap is nullptr"));
  }
}

Reply RedisCluster::Execute(const Command &cmd) {
  const ClusterArguments cluster{
      .slotmap = _slot_map,
      .use_master = _use_master,
      .timeout = _timeout,
      .passwd = &_passwd,
  };
  auto svr = GetServer(cmd, cluster);
  Redis exec(_slot_map->GetClusterName(), svr.first, svr.second, _timeout, _passwd);
  auto cmds = &cmd;
  int offset = BuildExecutor(exec, &cmds, 1, cluster);
  return exec.Execute(offset);
}

Future RedisCluster::ExecuteAsync(const Command &cmd, Callback cb) {
  const ClusterArguments cluster{
      .slotmap = _slot_map,
      .use_master = _use_master,
      .timeout = _timeout,
      .passwd = &_passwd,
  };
  auto svr = GetServer(cmd, cluster);
  Redis exec(_slot_map->GetClusterName(), svr.first, svr.second, _timeout, _passwd);
  auto cmds = &cmd;
  int offset = BuildExecutor(exec, &cmds, 1, cluster);
  return exec.ExecuteAsync(cb, offset);
}

void RedisCluster::Add(const Command &cmd) { _commands.push_back(&cmd); }

Reply RedisCluster::Execute() {
  if (_commands.empty()) {
    throw std::invalid_argument("redis commands is empty");
  }

  const ClusterArguments cluster{
      .slotmap = _slot_map,
      .use_master = _use_master,
      .timeout = _timeout,
      .passwd = &_passwd,
  };

  const auto mapper = BuildMapper(_commands, cluster);

  if (mapper.size() == 1) {
    const auto &svr = mapper.begin()->first;
    Redis exec(_slot_map->GetClusterName(), svr.first, svr.second, _timeout, _passwd);
    int offset = BuildExecutor(exec, _commands.data(), _commands.size(), cluster);
    return exec.Execute(offset);
  }

  Reply reply;
  auto reqs = BuildRequestResponse(mapper, _commands, cluster, &reply);

  brpc::ParallelChannel pchan;
  BuildPChan(pchan, mapper, move(reqs), &reply, cluster);

  brpc::Controller cntl;
  SetControllerAttr(&cntl, cluster);

  pchan.CallMethod(nullptr, &cntl, &dummy_req, &dummy_resp, nullptr);

  if (cntl.Failed()) {
    throw runtime_error(fmt::format("{}", cntl.ErrorText()));
  }

  return reply;
}

Future RedisCluster::ExecuteAsync(Callback cb) {
  if (_commands.empty()) {
    throw std::invalid_argument("redis commands is empty");
  }

  const ClusterArguments cluster{
      .slotmap = _slot_map,
      .use_master = _use_master,
      .timeout = _timeout,
      .passwd = &_passwd,
  };

  const auto mapper = BuildMapper(_commands, cluster);

  if (mapper.size() == 1) {
    const auto &svr = mapper.begin()->first;
    Redis exec(_slot_map->GetClusterName(), svr.first, svr.second, _timeout, _passwd);
    int offset = BuildExecutor(exec, _commands.data(), _commands.size(), cluster);
    return exec.ExecuteAsync(cb, offset);
  }

  auto async_data = make_shared<detail::AsyncData>();
  async_data->cb = cb;
  auto reply = &async_data->reply;

  auto reqs = BuildRequestResponse(mapper, _commands, cluster, reply);

  brpc::ParallelChannel pchan;
  BuildPChan(pchan, mapper, move(reqs), reply, cluster);

  auto cntl = &async_data->cntl;
  SetControllerAttr(cntl, cluster);

  pchan.CallMethod(nullptr, cntl, &dummy_req, &dummy_resp,
                   brpc::NewCallback(detail::Callback, weak_ptr{async_data}));

  return Future(async_data);
}

void RedisCluster::Reset() { _commands.clear(); }
}  // namespace redis
