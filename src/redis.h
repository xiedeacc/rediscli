#pragma once
#include <memory>
#include <string>
#include <vector>

#include "callback.h"
#include "command/command.h"
#include "future.h"
#include "reply.h"

namespace brpc {
class Channel;
}  // namespace brpc

namespace redis {
class Redis final {
 public:
  Redis(const std::string &cluster_name, const std::string &ip, int port, int timeout = 0,
        const std::string &passwd = "");
  Redis(std::shared_ptr<brpc::Channel>, int timeout = 0);
  Redis(const Redis &) = delete;
  Redis(Redis &&) = default;
  Redis &operator=(const Redis &) = delete;
  Redis &operator=(Redis &&) = default;

 public:
  Reply Execute(const Command &);
  Future ExecuteAsync(const Command &, Callback cb = nullptr);
  void Add(const Command &);
  void Add(Command &&) = delete;
  Reply Execute(int offset = 0);
  Future ExecuteAsync(Callback cb = nullptr, int offset = 0);
  void Reset();

 private:
  std::shared_ptr<brpc::Channel> _channel;
  std::vector<const Command *> _commands;
  int _timeout{0};
};
}  // namespace redis