#pragma once

#include <string_view>

#include "command.h"

namespace redis {
using namespace std::literals;

struct SLOTS {};

template <typename T>
class CLUSTER;

template <>
class CLUSTER<SLOTS> : public Command {
 public:
  CLUSTER() : Command(CommandType::System, ""sv) {
    AddComponents("CLUSTER");
    AddComponents("SLOTS");
  }
};
}  // namespace redis
