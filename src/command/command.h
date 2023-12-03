#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "src/detail/add_components.h"

namespace brpc {
class RedisRequest;
}

namespace redis {
enum class CommandType {
  Read,
  Write,
  System,
};

class Command {
 public:
  Command(CommandType, std::string_view key);
  virtual ~Command() = default;
  Command(const Command &) = delete;
  Command &operator=(const Command &) = delete;
  Command(Command &&) = default;
  Command &operator=(Command &&) = default;

 public:
  CommandType Type() const;
  const std::string &Key() const;
  void AddTo(brpc::RedisRequest *) const;
  std::string Text() const;

 public:
  template <typename T>
  Command &AddComponents(const T &val) {
    detail::AddComponents(_components, val);
    return *this;
  }

 private:
  CommandType _type;
  std::string _key;
  std::vector<std::string> _components;
};
}  // namespace redis
