#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class READONLY : public Command {
 public:
  READONLY() : Command(CommandType::System, "") { AddComponents("READONLY"); }
};
}  // namespace redis
