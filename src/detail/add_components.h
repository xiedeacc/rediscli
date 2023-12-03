#pragma once

#include <initializer_list>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "boost/lexical_cast.hpp"

namespace redis::detail {
inline void AddComponents(std::vector<std::string> &, const std::string_view &);
inline void AddComponents(std::vector<std::string> &, const std::string &);
template <typename T>
inline void AddComponents(std::vector<std::string> &, const T &);
template <typename K, typename V>
inline void AddComponents(std::vector<std::string> &, const std::pair<K, V> &);
template <typename T>
inline void AddComponents(std::vector<std::string> &, const std::vector<T> &);
template <typename T>
inline void AddComponents(std::vector<std::string> &, const std::set<T> &);
template <typename T>
inline void AddComponents(std::vector<std::string> &,
                          const std::initializer_list<T> &);
template <typename K, typename V>
inline void AddComponents(std::vector<std::string> &, const std::map<K, V> &);
template <typename K, typename V>
inline void AddComponents(std::vector<std::string> &,
                          const std::unordered_map<K, V> &);

inline void AddComponents(std::vector<std::string> &v,
                          const std::string_view &val) {
  v.emplace_back(val);
}

inline void AddComponents(std::vector<std::string> &v, const std::string &val) {
  v.emplace_back(val);
}

template <typename T>
inline void AddComponents(std::vector<std::string> &v, const T &val) {
  v.emplace_back(boost::lexical_cast<std::string>(val));
}

template <typename K, typename V>
inline void AddComponents(std::vector<std::string> &v,
                          const std::pair<K, V> &val) {
  AddComponents(v, val.first);
  AddComponents(v, val.second);
}

template <typename T>
inline void AddComponents(std::vector<std::string> &v,
                          const std::vector<T> &val) {
  for (auto &elem : val) {
    AddComponents(v, elem);
  }
}

template <typename T>
inline void AddComponents(std::vector<std::string> &v, const std::set<T> &val) {
  for (auto &elem : val) {
    AddComponents(v, elem);
  }
}

template <typename T>
inline void AddComponents(std::vector<std::string> &v,
                          const std::initializer_list<T> &val) {
  for (auto &elem : val) {
    AddComponents(v, elem);
  }
}

template <typename K, typename V>
inline void AddComponents(std::vector<std::string> &v,
                          const std::map<K, V> &m) {
  for (auto &item : m) {
    AddComponents(v, item);
  }
}

template <typename K, typename V>
inline void AddComponents(std::vector<std::string> &v,
                          const std::unordered_map<K, V> &m) {
  for (auto &item : m) {
    AddComponents(v, item);
  }
}

}  // namespace redis::detail
