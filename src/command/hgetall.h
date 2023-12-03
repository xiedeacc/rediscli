#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class HGETALL : public Command {
 public:
  HGETALL(std::string_view key) : Command(CommandType::Read, key) {
    AddComponents("HGETALL");
    AddComponents(key);
  }
};
}  // namespace redis
