#pragma once
#include <string_view>
#include <type_traits>

namespace redis::detail {
template <typename T>
using is_key = std::is_convertible<T, std::string_view>;

template <typename T>
constexpr bool is_key_v = std::is_convertible_v<T, std::string_view>;

template <typename T>
using enable_if_is_key_t = std::enable_if_t<is_key_v<T>>;

template <typename T>
constexpr bool is_value_v = std::is_integral_v<T> || std::is_floating_point_v<T> || is_key_v<T>;

template <typename T, typename = void>
struct value_type_is_key : std::false_type {};

template <typename T>
struct value_type_is_key<T, enable_if_is_key_t<typename T::value_type>> : std::true_type {};

template <typename T>
constexpr bool value_type_is_key_v = value_type_is_key<T>::value;

template <typename T, typename = void>
struct value_type_is_pair : std::false_type {};

template <typename T>
struct value_type_is_pair<T, std::enable_if_t<is_key_v<typename T::value_type::first_type> &&
                                              is_value_v<typename T::value_type::second_type>>>
    : std::true_type {};

template <typename T>
constexpr bool value_type_is_pair_v = value_type_is_pair<T>::value;
}  // namespace redis::detail