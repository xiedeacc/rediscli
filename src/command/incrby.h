#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class INCRBY : public Command {
 public:
  INCRBY(std::string_view key, int increment) : Command(CommandType::Write, key) {
    AddComponents("INCRBY");
    AddComponents(key);
    AddComponents(increment);
  }
};
}  // namespace redis
