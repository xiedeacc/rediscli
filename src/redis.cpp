#include "redis.h"

#include <brpc/channel.h>
#include <brpc/controller.h>
#include <brpc/redis.h>
#include <fmt/format.h>

#include <stdexcept>

#include "detail/async_data.h"
#include "detail/redis_channel_cache.h"
using namespace std;

namespace {
void SetControllerAttr(brpc::Controller *cntl, int timeout) {
  if (timeout > 0) {
    cntl->set_timeout_ms(timeout);
  }
}
}  // namespace

namespace redis {
Redis::Redis(const string &cluster_name, const string &ip, int port, int timeout,
             const string &passwd) {
  _channel = detail::RedisChannelCache::Instance().GetChannel(cluster_name, ip, port, passwd);
  if (_channel == nullptr) {
    throw runtime_error(fmt::format("failed to get redis channel {}:{}", ip, port));
  }
  _timeout = timeout;
}

Redis::Redis(shared_ptr<brpc::Channel> chan, int timeout) : _channel{chan}, _timeout{timeout} {
  if (_channel == nullptr) {
    throw runtime_error(fmt::format("redis channel is nullptr"));
  }
}

Reply Redis::Execute(const Command &cmd) {
  Reply reply;

  brpc::Controller cntl;
  SetControllerAttr(&cntl, _timeout);

  brpc::RedisRequest req;
  cmd.AddTo(&req);

  auto resp = reply.AddResponse();

  _channel->CallMethod(nullptr, &cntl, &req, resp, nullptr);

  if (cntl.Failed()) {
    throw runtime_error(fmt::format("{}", cntl.ErrorText()));
  }
  return reply;
}

Future Redis::ExecuteAsync(const Command &cmd, Callback cb) {
  auto async_data = make_shared<detail::AsyncData>();
  async_data->cb = cb;

  auto &reply = async_data->reply;

  auto cntl = &async_data->cntl;
  SetControllerAttr(cntl, _timeout);

  brpc::RedisRequest req;
  cmd.AddTo(&req);

  auto resp = reply.AddResponse();

  _channel->CallMethod(nullptr, cntl, &req, resp,
                       brpc::NewCallback(detail::Callback, weak_ptr{async_data}));
  return Future(async_data);
}

void Redis::Add(const Command &cmd) { _commands.push_back(&cmd); }

Reply Redis::Execute(int offset) {
  if (_commands.empty()) {
    throw invalid_argument("redis commands is empty");
  }

  Reply reply;

  brpc::Controller cntl;
  SetControllerAttr(&cntl, _timeout);

  brpc::RedisRequest req;
  for (auto cmd : _commands) {
    cmd->AddTo(&req);
  }

  auto resp = reply.AddResponse(offset);

  _channel->CallMethod(nullptr, &cntl, &req, resp, nullptr);

  if (cntl.Failed()) {
    throw runtime_error(fmt::format("{}", cntl.ErrorText()));
  }
  return reply;
}

Future Redis::ExecuteAsync(Callback cb, int offset) {
  if (_commands.empty()) {
    throw invalid_argument("redis commands is empty");
  }

  auto async_data = make_shared<detail::AsyncData>();
  async_data->cb = cb;

  auto &reply = async_data->reply;

  auto cntl = &async_data->cntl;
  SetControllerAttr(cntl, _timeout);

  brpc::RedisRequest req;
  for (auto cmd : _commands) {
    cmd->AddTo(&req);
  }

  auto resp = reply.AddResponse(offset);

  _channel->CallMethod(nullptr, cntl, &req, resp,
                       brpc::NewCallback(detail::Callback, weak_ptr{async_data}));
  return Future(async_data);
}

void Redis::Reset() { _commands.clear(); }
}  // namespace redis
