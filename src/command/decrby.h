#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class DECRBY : public Command {
 public:
  DECRBY(std::string_view key, int decrement) : Command(CommandType::Write, key) {
    AddComponents("DECRBY");
    AddComponents(key);
    AddComponents(decrement);
  }
};
}  // namespace redis
