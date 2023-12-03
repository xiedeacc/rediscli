#pragma once
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "boost/lexical_cast.hpp"
#include "brpc/redis.h"
#include "fmt/format.h"

namespace brpc {
inline auto format_as(RedisReplyType e) { return fmt::underlying(e); }
}  // namespace brpc

namespace redis::detail {
using namespace std::literals;

template <typename T>
inline void GetReply(const brpc::RedisReply &, T &);
inline void GetReply(const brpc::RedisReply &, std::string &);
inline void GetReply(const brpc::RedisReply &, std::string_view &);
template <typename T, size_t... I>
inline void GetTupleHelper(const brpc::RedisReply &, T &,
                           const std::index_sequence<I...> &);
template <typename... Args>
inline void GetReply(const brpc::RedisReply &, std::tuple<Args...> &);
template <typename T>
inline void GetReply(const brpc::RedisReply &, std::vector<T> &);
template <typename T>
inline void GetReply(const brpc::RedisReply &, std::set<T> &);
template <typename T>
inline void GetReply(const brpc::RedisReply &, std::unordered_set<T> &);
template <typename K, typename V>
inline void GetReply(const brpc::RedisReply &, std::unordered_map<K, V> &);
template <typename K, typename V>
inline void GetReply(const brpc::RedisReply &, std::map<K, V> &);

template <typename T>
inline void GetReply(const brpc::RedisReply &reply, T &val) {
  switch (reply.type()) {
    case brpc::REDIS_REPLY_STRING: {
      auto data = reply.data();
      val = boost::lexical_cast<T>(std::string_view(data.data(), data.size()));
    } break;
    case brpc::REDIS_REPLY_ARRAY: {
      throw std::invalid_argument("can not convert array to scale");
    } break;
    case brpc::REDIS_REPLY_INTEGER: {
      val = boost::lexical_cast<T>(reply.integer());
    } break;
    case brpc::REDIS_REPLY_NIL: {
      val = {};
    } break;
    case brpc::REDIS_REPLY_STATUS: {
      std::string_view data = "0"sv;
      if (reply.data() == "OK") {
        data = "1"sv;
      }
      val = boost::lexical_cast<T>(data);
    } break;
    case brpc::REDIS_REPLY_ERROR: {
      throw std::runtime_error(fmt::format("{}", reply.error_message()));
    } break;
  }
}

inline void GetReply(const brpc::RedisReply &reply, std::string &val) {
  switch (reply.type()) {
    case brpc::REDIS_REPLY_STRING: {
      auto data = reply.data();
      val.assign(data.data(), data.size());
    } break;
    case brpc::REDIS_REPLY_ARRAY: {
      throw std::invalid_argument("can not convert array to string");
    } break;
    case brpc::REDIS_REPLY_INTEGER: {
      val = boost::lexical_cast<std::string>(reply.integer());
    } break;
    case brpc::REDIS_REPLY_NIL: {
      val = ""s;
    } break;
    case brpc::REDIS_REPLY_STATUS: {
      auto data = reply.data();
      val.assign(data.data(), data.size());
    } break;
    case brpc::REDIS_REPLY_ERROR: {
      throw std::runtime_error(fmt::format("{}", reply.error_message()));
    } break;
  }
}

inline void GetReply(const brpc::RedisReply &reply, std::string_view &val) {
  switch (reply.type()) {
    case brpc::REDIS_REPLY_STRING: {
      auto data = reply.data();
      val = std::string_view(data.data(), data.size());
    } break;
    case brpc::REDIS_REPLY_ARRAY: {
      throw std::invalid_argument("can not convert array to string_view");
    } break;
    case brpc::REDIS_REPLY_INTEGER: {
      throw std::invalid_argument("can not convert integer to string_view");
    } break;
    case brpc::REDIS_REPLY_NIL: {
      val = ""sv;
    } break;
    case brpc::REDIS_REPLY_STATUS: {
      auto data = reply.data();
      val = std::string_view(data.data(), data.size());
    } break;
    case brpc::REDIS_REPLY_ERROR: {
      throw std::runtime_error(fmt::format("{}", reply.error_message()));
    } break;
  }
}

template <typename T, size_t... I>
inline void GetTupleHelper(const brpc::RedisReply &reply, T &val,
                           const std::index_sequence<I...> &) {
  ((GetReply(reply[I], std::get<I>(val))), ...);
}

template <typename... Args>
inline void GetReply(const brpc::RedisReply &reply, std::tuple<Args...> &val) {
  if (sizeof...(Args) != reply.size()) {
    throw std::invalid_argument(
        fmt::format("reply size is not equal to tuple size: expect {}, get {}",
                    sizeof...(Args), reply.size()));
  }
  GetTupleHelper(reply, val, std::index_sequence_for<Args...>{});
}

template <typename T>
inline void GetReply(const brpc::RedisReply &reply, std::vector<T> &val) {
  if (reply.type() == brpc::REDIS_REPLY_NIL) {
    return;
  }
  if (reply.type() == brpc::REDIS_REPLY_ERROR) {
    throw std::runtime_error(fmt::format("{}", reply.error_message()));
  }
  if (reply.type() != brpc::REDIS_REPLY_ARRAY) {
    throw std::invalid_argument(
        fmt::format("can not convert {} to vector", reply.type()));
  }
  for (size_t i = 0; i < reply.size(); ++i) {
    T elem;
    GetReply(reply[i], elem);
    val.emplace_back(std::move(elem));
  }
}

template <typename T>
inline void GetReply(const brpc::RedisReply &reply, std::set<T> &val) {
  if (reply.type() == brpc::REDIS_REPLY_NIL) {
    return;
  }
  if (reply.type() == brpc::REDIS_REPLY_ERROR) {
    throw std::runtime_error(fmt::format("{}", reply.error_message()));
  }
  if (reply.type() != brpc::REDIS_REPLY_ARRAY) {
    throw std::invalid_argument(
        fmt::format("can not convert {} to set", reply.type()));
  }
  for (size_t i = 0; i < reply.size(); ++i) {
    T elem;
    GetReply(reply[i], elem);
    val.emplace(std::move(elem));
  }
}

template <typename T>
inline void GetReply(const brpc::RedisReply &reply,
                     std::unordered_set<T> &val) {
  if (reply.type() == brpc::REDIS_REPLY_NIL) {
    return;
  }
  if (reply.type() == brpc::REDIS_REPLY_ERROR) {
    throw std::runtime_error(fmt::format("{}", reply.error_message()));
  }
  if (reply.type() != brpc::REDIS_REPLY_ARRAY) {
    throw std::invalid_argument(
        fmt::format("can not convert {} to unordered_set", reply.type()));
  }
  for (size_t i = 0; i < reply.size(); ++i) {
    T elem;
    GetReply(reply[i], elem);
    val.emplace(std::move(elem));
  }
}

template <typename K, typename V>
inline void GetReply(const brpc::RedisReply &reply,
                     std::unordered_map<K, V> &val) {
  if (reply.type() == brpc::REDIS_REPLY_NIL) {
    return;
  }
  if (reply.type() == brpc::REDIS_REPLY_ERROR) {
    throw std::runtime_error(fmt::format("{}", reply.error_message()));
  }
  if (reply.type() != brpc::REDIS_REPLY_ARRAY) {
    throw std::invalid_argument(
        fmt::format("can not convert {} to unordered_map", reply.type()));
  }
  if (reply.size() % 2u != 0) {
    throw std::invalid_argument("reply.size() is not even");
  }
  for (size_t i = 0; i < reply.size(); i += 2) {
    K k;
    V v;
    GetReply(reply[i], k);
    GetReply(reply[i + 1], v);
    val.emplace(std::move(k), std::move(v));
  }
}

template <typename K, typename V>
inline void GetReply(const brpc::RedisReply &reply, std::map<K, V> &val) {
  if (reply.type() == brpc::REDIS_REPLY_NIL) {
    return;
  }
  if (reply.type() == brpc::REDIS_REPLY_ERROR) {
    throw std::runtime_error(fmt::format("{}", reply.error_message()));
  }
  if (reply.type() != brpc::REDIS_REPLY_ARRAY) {
    throw std::invalid_argument(
        fmt::format("can not convert {} to map", reply.type()));
  }
  if (reply.size() % 2u != 0) {
    throw std::invalid_argument("reply.size() is not even");
  }
  for (size_t i = 0; i < reply.size(); i += 2) {
    K k;
    V v;
    GetReply(reply[i], k);
    GetReply(reply[i + 1], v);
    val.emplace(std::move(k), std::move(v));
  }
}
}  // namespace redis::detail
