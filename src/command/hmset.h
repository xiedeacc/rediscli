#pragma once

#include <initializer_list>
#include <string_view>

#include "command.h"
#include "src/detail/command_type_traits.h"

namespace redis {
class HMSET : public Command {
 public:
  template <typename T>
  HMSET(std::string_view key, std::string_view field, const T& value)
      : Command(CommandType::Write, key) {
    static_assert(detail::is_value_v<T>, "invalid value type");
    AddComponents("HMSET");
    AddComponents(key);
    AddComponents(field);
    AddComponents(value);
  }
  template <typename K, typename V>
  HMSET(std::string_view key,
        std::initializer_list<std::pair<K, V>> field_values)
      : Command(CommandType::Write, key) {
    static_assert(detail::is_key_v<K>, "invalid field type");
    static_assert(detail::is_value_v<V>, "invalid value type");
    AddComponents("HMSET");
    AddComponents(key);
    AddComponents(field_values);
  }
  template <typename Range>
  HMSET(std::string_view key, const Range& field_values)
      : Command(CommandType::Write, key) {
    static_assert(detail::value_type_is_pair_v<Range>, "invalid range type");
    AddComponents("HMSET");
    AddComponents(key);
    AddComponents(field_values);
  }
};
}  // namespace redis
