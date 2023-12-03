#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class HKEYS : public Command {
 public:
  HKEYS(std::string_view key) : Command(CommandType::Read, key) {
    AddComponents("HKEYS");
    AddComponents(key);
  }
};
}  // namespace redis
