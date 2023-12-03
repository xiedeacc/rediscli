#include "redis_channel_cache.h"

#include "brpc/channel.h"
#include "brpc/policy/redis_authenticator.h"
#include "brpc/redis.h"
#include "fmt/format.h"
#include "passwd_map.h"
#include "src/nlog.h"

namespace redis::detail {

std::shared_ptr<brpc::Channel> RedisChannelCache::GetChannel(
    const std::string& cluster_name, const std::string& ip, int port,
    std::string passwd) {
  const std::string key = fmt::format("{}:{}", ip, port);

  {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    auto it = channels_.find(key);
    if (it != channels_.end()) {
      return it->second;
    }
  }

  boost::unique_lock<boost::shared_mutex> lock(mutex_);
  auto it = channels_.find(key);
  if (it != channels_.end()) {
    return it->second;
  }

  auto channel = std::make_shared<brpc::Channel>();
  brpc::ChannelOptions options;
  options.protocol = brpc::PROTOCOL_REDIS;

  if (passwd.empty()) {
    passwd = PasswdMap::get_password(cluster_name);
  }
  if (!passwd.empty()) {
    brpc::policy::RedisAuthenticator* auth =
        new brpc::policy::RedisAuthenticator(passwd);
    options.auth = auth;
  }
  if (channel->Init(key.data(), &options) == 0) {
    LOG_DEBUG("init brpc channel success, key:{}", key);
    channels_.emplace(key, channel);
  } else {
    LOG_CRITICAL("init brpc channel failed, key:{}", key);
    return nullptr;
  }
  return channel;
}

bool RedisChannelCache::RegisterChannel(const std::string& cluster_name,
                                        const std::string& ip, int port,
                                        const std::string& passwd) {
  if (GetChannel(cluster_name, ip, port, passwd)) {
    return true;
  }
  return false;
}

}  // namespace redis::detail
