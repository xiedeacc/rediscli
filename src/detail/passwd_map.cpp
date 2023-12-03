#include "passwd_map.h"

#include <fstream>
#include <unordered_map>

#include "flags.h"
#include "src/nlog.h"

// 本来是一个从端口到密码的映射，单例
namespace redis::detail {

static const char* kPasswordFile = "/data/secret/.nads_redis_auth.env";

struct RedisPasswdMapImp {
  static RedisPasswdMapImp& get() {
    static RedisPasswdMapImp s;
    return s;
  }

  RedisPasswdMapImp() {
    // TODO load file
    std::ifstream ifs(kPasswordFile, std::ios::in | std::ios::binary);
    std::string line;
    int line_num = 0;
    while (std::getline(ifs, line)) {
      ++line_num;
      auto index = line.find(':');
      if (index == std::string::npos || index == 0 ||
          index == line.size() - 1) {
        if (FLAGS_enable_redis_auth_fatal_log) {
          LOG_CRITICAL("File {} line {} is invalid", kPasswordFile, line_num);
        } else {
          LOG_WARN("File {} line {} is invalid", kPasswordFile, line_num);
        }
        continue;
      }
      cluster_to_passwd_.emplace(line.substr(0, index), line.substr(index + 1));
    }
    LOG_INFO("Load redis pwd file complete");
    for (const auto& item : cluster_to_passwd_) {
      // LOG_DEBUG("Load redis pwd file cluster name:{}, password:{}",
      // item.first, item.second);
    }
    ifs.close();
  }

  std::unordered_map<std::string, std::string> cluster_to_passwd_;
};

std::string PasswdMap::get_password(const std::string& cluster_name) {
  auto& map = RedisPasswdMapImp::get().cluster_to_passwd_;
  auto it = map.find(cluster_name);
  if (it == map.end()) {
    if (FLAGS_enable_redis_auth_fatal_log) {
      LOG_CRITICAL("Password for cluster name: {}, not found", cluster_name);
    } else {
      LOG_WARN("Password for cluster name: {}, not found", cluster_name);
    }
    return "";
  }
  return it->second;
}

}  // namespace redis::detail
