#include "async_data.h"

#include <stdexcept>

#include "fmt/format.h"
using namespace std;

namespace redis::detail {
void Callback(std::weak_ptr<AsyncData> ptr) {
  auto p = ptr.lock();
  if (!p) {
    return;
  }
  if (p->cb) {
    try {
      auto &cntl = p->cntl;
      if (cntl.Failed()) {
        throw runtime_error(fmt::format("{}", cntl.ErrorText()));
      }
      p->cb(p->reply);
    } catch (...) {
      p->ex_ptr = current_exception();
    }
  }
  p->has_finish.store(true, memory_order_release);
}
}  // namespace redis::detail
