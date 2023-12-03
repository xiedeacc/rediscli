#pragma once

#include <initializer_list>
#include <string_view>

#include "command.h"
#include "src/detail/command_type_traits.h"

namespace redis {
class HMGET : public Command {
 public:
  HMGET(std::string_view key, std::initializer_list<std::string_view> fields)
      : Command(CommandType::Read, key) {
    AddComponents("HMGET");
    AddComponents(key);
    AddComponents(fields);
  }
  template <typename Range,
            typename = detail::enable_if_is_key_t<typename Range::value_type>>
  HMGET(std::string_view key, const Range& fields)
      : Command(CommandType::Read, key) {
    AddComponents("HMGET");
    AddComponents(key);
    AddComponents(fields);
  }
};
}  // namespace redis
