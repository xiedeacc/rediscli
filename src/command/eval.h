#pragma once

#include <initializer_list>
#include <string_view>

#include "command.h"

namespace redis {
class EVAL : public Command {
 public:
  template <typename... Args>
  EVAL(std::string_view script, std::initializer_list<std::string_view> keys,
       Args... args)
      : Command(CommandType::Write, *keys.begin()) {
    AddComponents("EVAL");
    AddComponents(script);
    AddComponents(keys.size());
    AddComponents(keys);
    (AddComponents(args), ...);
  }

  template <typename... Args>
  EVAL &AddArgs(Args &&...args) {
    (AddComponents(std::forward<Args>(args)), ...);
    return *this;
  }
};
}  // namespace redis
