#pragma once

#include <initializer_list>
#include <string_view>

#include "command.h"
#include "src/detail/command_type_traits.h"

namespace redis {
class HSET : public Command {
 public:
  template <typename T>
  HSET(std::string_view key, std::string_view field, const T& value)
      : Command(CommandType::Write, key) {
    static_assert(detail::is_value_v<T>, "invalid value type");
    AddComponents("HSET");
    AddComponents(key);
    AddComponents(field);
    AddComponents(value);
  }
};
}  // namespace redis
