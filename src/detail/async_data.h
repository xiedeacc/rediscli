#pragma once

#include <atomic>
#include <exception>
#include <memory>

#include "brpc/controller.h"
#include "src/callback.h"
#include "src/reply.h"

namespace redis::detail {
struct AsyncData {
  brpc::Controller cntl;
  Reply reply;
  redis::Callback cb;
  std::exception_ptr ex_ptr;
  std::atomic_bool has_finish{false};
};
void Callback(std::weak_ptr<AsyncData>);
}  // namespace redis::detail
