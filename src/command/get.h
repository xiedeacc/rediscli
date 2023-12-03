#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class GET : public Command {
 public:
  GET(std::string_view key) : Command(CommandType::Read, key) {
    AddComponents("GET");
    AddComponents(key);
  }
};
}  // namespace redis
