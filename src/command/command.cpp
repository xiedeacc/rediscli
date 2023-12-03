#include "command.h"

#include "boost/algorithm/string/join.hpp"
#include "brpc/redis.h"
#include "butil/strings/string_piece.h"

using namespace std;

namespace redis {
Command::Command(CommandType type, string_view key) : _type{type}, _key{key} {}

CommandType Command::Type() const { return _type; }

const string &Command::Key() const { return _key; }

void Command::AddTo(brpc::RedisRequest *req) const {
  if (_components.size() != 1) {
    vector<butil::StringPiece> args{_components.begin(), _components.end()};
    req->AddCommandByComponents(args.data(), args.size());
  } else {
    req->AddCommand(_components[0]);
  }
}

string Command::Text() const { return boost::join(_components, " "); }

}  // namespace redis
