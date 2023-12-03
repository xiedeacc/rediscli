#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class HINCRBY : public Command {
 public:
  HINCRBY(std::string_view key) : Command(CommandType::Write, key) {
    AddComponents("HINCRBY");
    AddComponents(key);
  }
};
}  // namespace redis
