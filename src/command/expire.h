#pragma once

#include <string_view>

#include "command.h"

namespace redis {
class EXPIRE : public Command {
 public:
  EXPIRE(std::string_view key, long seconds) : Command(CommandType::Write, key) {
    AddComponents("EXPIRE");
    AddComponents(key);
    AddComponents(seconds);
  }
  EXPIRE &NX() {
    AddComponents("NX");
    return *this;
  }
  EXPIRE &XX() {
    AddComponents("XX");
    return *this;
  }
  EXPIRE &GT() {
    AddComponents("GT");
    return *this;
  }
  EXPIRE &LT() {
    AddComponents("LT");
    return *this;
  }
};
}  // namespace redis
