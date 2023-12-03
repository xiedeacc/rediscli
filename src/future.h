#pragma once
#include <memory>

namespace brpc {
class Controller;
}

namespace redis {
class Reply;
namespace detail {
struct AsyncData;
}
class Future final {
 public:
  Future() = default;
  Future(std::shared_ptr<detail::AsyncData>);
  ~Future();
  Future(const Future &) = delete;
  Future &operator=(const Future &) = delete;
  Future(Future &&) = default;
  Future &operator=(Future &&) = default;

 public:
  Reply *Get() const;
  brpc::Controller *GetController() const;
  void Wait() const;
  bool Valid() const noexcept;

 private:
  std::shared_ptr<detail::AsyncData> _data;
};
}  // namespace redis