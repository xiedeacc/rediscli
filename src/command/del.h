#pragma once

#include <initializer_list>
#include <string_view>

#include "command.h"
#include "src/detail/command_type_traits.h"

namespace redis {
class DEL : public Command {
 public:
  DEL(std::string_view key) : Command(CommandType::Write, key) {
    AddComponents("DEL");
    AddComponents(key);
  }
  DEL(std::initializer_list<std::string_view> keys)
      : Command(CommandType::Write, *keys.begin()) {
    AddComponents("DEL");
    AddComponents(keys);
  }
  template <typename Range,
            typename = detail::enable_if_is_key_t<typename Range::value_type>>
  DEL(const Range &keys) : Command(CommandType::Write, *keys.begin()) {
    AddComponents("DEL");
    AddComponents(keys);
  }
};
}  // namespace redis
