#include "future.h"

#include <fmt/format.h>

#include <stdexcept>

#include "detail/async_data.h"
using namespace std;

namespace redis {
Future::Future(shared_ptr<detail::AsyncData> p) : _data{p} {}
Future::~Future() = default;

Reply *Future::Get() const {
  Wait();
  auto &cntl = _data->cntl;
  if (cntl.Failed()) {
    throw runtime_error(fmt::format("{}", cntl.ErrorText()));
  }
  return &_data->reply;
}

brpc::Controller *Future::GetController() const {
  if (!Valid()) {
    throw runtime_error("invalid redis future");
  }
  return &_data->cntl;
}

void Future::Wait() const {
  if (!Valid()) {
    throw runtime_error("invalid redis future");
  }
  if (!_data->has_finish.load(memory_order_acquire)) {
    brpc::Join(_data->cntl.call_id());
  }
  if (_data->cb && _data->ex_ptr) {
    rethrow_exception(_data->ex_ptr);
  }
}

bool Future::Valid() const noexcept { return _data != nullptr; }
}  // namespace redis
