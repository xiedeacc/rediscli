#pragma once
#include <functional>

#include "reply.h"

namespace redis {
using Callback = std::function<void(const Reply &)>;
}