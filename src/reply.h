#pragma once

#include <utility>
#include <vector>

#include "brpc/redis.h"
#include "src/detail/get_reply.h"

namespace redis {
class Reply final {
 public:
  Reply() = default;
  ~Reply() = default;
  Reply(const Reply &) = delete;
  Reply(Reply &&) = default;
  Reply &operator=(const Reply &) = delete;
  Reply &operator=(Reply &&) = default;

 public:
  template <typename T>
  T Get(int i = 0) const {
    T val;
    detail::GetReply(GetRedisReply(i), val);
    return val;
  }

  const brpc::RedisReply &GetRedisReply(int i = 0) const;

 public:
  brpc::RedisResponse *GetResponse(int idx);
  brpc::RedisResponse *AddResponse(int offset = 0) noexcept;
  brpc::RedisResponse *AddResponse(const std::vector<int> &command_indexes,
                                   int offset = 0) noexcept;

 private:
  struct ChannelData {
    brpc::RedisResponse resp;
    int offset{0};  // 1 for readonly channel, otherwise 0
  };
  std::vector<ChannelData> _resps;
  std::vector<std::pair<int, int>> _mapping;
};
}  // namespace redis
