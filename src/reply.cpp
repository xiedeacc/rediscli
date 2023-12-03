#include "src/reply.h"

#include <stdexcept>

#include "fmt/format.h"
using namespace std;

namespace redis {
brpc::RedisResponse *Reply::GetResponse(int idx) {
  return &(_resps.at(idx).resp);
}

brpc::RedisResponse *Reply::AddResponse(int offset) noexcept {
  ChannelData data;
  data.offset = offset;
  _resps.emplace_back(move(data));
  return &(_resps.back().resp);
}

brpc::RedisResponse *Reply::AddResponse(const vector<int> &command_indexes,
                                        int offset) noexcept {
  ChannelData data;
  data.offset = offset;
  _resps.emplace_back(move(data));

  auto resp_idx = _resps.size() - 1;

  for (size_t i = 0; i < command_indexes.size(); ++i) {
    auto idx = command_indexes[i];
    if (_mapping.size() <= idx) {
      _mapping.resize(idx + 1);
    }
    _mapping[idx] = pair<int, int>{resp_idx, i};
  }

  return &(_resps.back().resp);
}

const brpc::RedisReply &Reply::GetRedisReply(int i) const {
  auto m = std::pair{0, i};
  if (!_mapping.empty()) {
    m = _mapping.at(i);
  }
  auto &chan_data = _resps.at(m.first);
  auto idx = m.second + chan_data.offset;
  if (chan_data.resp.reply_size() <= idx) {
    throw out_of_range(fmt::format("invalid command index {}", i));
  }
  return chan_data.resp.reply(idx);
}
}  // namespace redis
