#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class HVALS : public Command {
 public:
  HVALS(std::string_view key) : Command(CommandType::Read, key) {
    AddComponents("HVALS");
    AddComponents(key);
  }
};
}  // namespace redis
