#pragma once

#include <string_view>

#include "command.h"
#include "src/detail/command_type_traits.h"

namespace redis {
class SETNX : public Command {
 public:
  template <typename T>
  SETNX(std::string_view key, const T &value)
      : Command(CommandType::Write, key) {
    static_assert(detail::is_value_v<T>, "invalid value type");
    AddComponents("SETNX");
    AddComponents(key);
    AddComponents(value);
  }
};
}  // namespace redis
