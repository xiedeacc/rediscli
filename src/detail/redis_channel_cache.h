#pragma once
#include <memory>
#include <unordered_map>

#include "boost/thread/shared_mutex.hpp"

namespace brpc {
class Channel;
}

namespace redis::detail {

class RedisChannelCache {
 public:
  std::shared_ptr<brpc::Channel> GetChannel(const std::string& cluster_name,
                                            const std::string& ip, int port,
                                            std::string passwd = "");
  bool RegisterChannel(const std::string& cluster_name, const std::string& ip,
                       int port, const std::string& passwd = "");
  static RedisChannelCache& Instance() {
    static RedisChannelCache _instance;
    return _instance;
  }

 private:
  std::unordered_map<std::string, std::shared_ptr<brpc::Channel>> channels_;
  boost::shared_mutex mutex_;
};

}  // namespace redis::detail
