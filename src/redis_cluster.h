#pragma once
#include <string>
#include <vector>

#include "callback.h"
#include "command/command.h"
#include "future.h"
#include "reply.h"

namespace redis {
class SlotMap;
class RedisCluster final {
 public:
  RedisCluster(const std::string &cluster_name, int timeout = 0);
  RedisCluster(SlotMap *, int timeout = 0, const std::string &passwd = "");
  RedisCluster(const RedisCluster &) = delete;
  RedisCluster(RedisCluster &&) = default;
  RedisCluster &operator=(const RedisCluster &) = delete;
  RedisCluster &operator=(RedisCluster &&) = default;

 public:
  Reply Execute(const Command &);
  Future ExecuteAsync(const Command &, Callback cb = nullptr);
  void Add(const Command &);
  void Add(Command &&) = delete;
  Reply Execute();
  Future ExecuteAsync(Callback cb = nullptr);
  void Reset();

 private:
  std::vector<const Command *> _commands;
  SlotMap *_slot_map{nullptr};
  int _timeout{0};
  std::string _passwd;
  bool _use_master{false};
};
}  // namespace redis