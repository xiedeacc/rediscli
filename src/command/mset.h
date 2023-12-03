#pragma once

#include <initializer_list>
#include <string_view>

#include "command.h"
#include "src/detail/command_type_traits.h"

namespace redis {
class MSET : public Command {
 public:
  template <typename K, typename V>
  MSET(std::initializer_list<std::pair<K, V>> pairs)
      : Command(CommandType::Write, (keys.begin())->first) {
    static_assert(detail::is_value_v<V>, "invalid value type");
    AddComponents("MSET");
    AddComponents(pairs);
  }
  template <typename Range>
  MSET(const Range& r) : Command(CommandType::Write, (r.begin())->first) {
    static_assert(detail::value_type_is_pair_v<Range>, "invalid range type");
    AddComponents("MSET");
    AddComponents(r);
  }
};
}  // namespace redis
