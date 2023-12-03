#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class HGET : public Command {
 public:
  HGET(std::string_view key, std::string_view field) : Command(CommandType::Read, key) {
    AddComponents("HGET");
    AddComponents(key);
    AddComponents(field);
  }
};
}  // namespace redis
