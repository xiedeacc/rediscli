#pragma once

#include <initializer_list>
#include <string_view>

#include "command.h"

namespace redis {
class MGET : public Command {
 public:
  MGET(std::initializer_list<std::string_view> keys) : Command(CommandType::Read, *keys.begin()) {
    AddComponents("MGET");
    AddComponents(keys);
  }
  template <typename Range>
  MGET(const Range& keys) : Command(CommandType::Read, *keys.begin()) {
    AddComponents("MGET");
    AddComponents(keys);
  }
};
}  // namespace redis
