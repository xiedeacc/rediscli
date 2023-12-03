#pragma once

#include <string_view>

#include "command.h"
#include "src/detail/command_type_traits.h"

namespace redis {
class SET : public Command {
 public:
  template <typename T>
  SET(std::string_view key, const T &value) : Command(CommandType::Write, key) {
    static_assert(detail::is_value_v<T>, "invalid value type");
    AddComponents("SET");
    AddComponents(key);
    AddComponents(value);
  }
  SET &EX(int sec) {
    AddComponents("EX");
    AddComponents(sec);
    return *this;
  }
  SET &EXAT(long timestamp) {
    AddComponents("EXAT");
    AddComponents(timestamp);
    return *this;
  }
  SET &NX() {
    AddComponents("NX");
    return *this;
  }
  SET &XX() {
    AddComponents("XX");
    return *this;
  }
};
}  // namespace redis
