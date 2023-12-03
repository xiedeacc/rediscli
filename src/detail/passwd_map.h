#pragma once

#include <string>

// 本来是一个从端口到密码的映射，单例
namespace redis::detail {
class PasswdMap {
 public:
  static std::string get_password(const std::string& cluster_name);
};
}  // namespace redis::detail
