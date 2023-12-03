#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class SMEMBERS : public Command {
 public:
  SMEMBERS(std::string_view key) : Command(CommandType::Read, key) {
    AddComponents("SMEMBERS");
    AddComponents(key);
  }
};
}  // namespace redis
