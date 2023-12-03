#pragma once

#include <string_view>

#include "command.h"
#include "src/detail/command_type_traits.h"

namespace redis {
class SETEX : public Command {
 public:
  template <typename T>
  SETEX(std::string_view key, int sec, const T &value)
      : Command(CommandType::Write, key) {
    static_assert(detail::is_value_v<T>, "invalid value type");
    AddComponents("SETEX");
    AddComponents(key);
    AddComponents(sec);
    AddComponents(value);
  }
};
}  // namespace redis
