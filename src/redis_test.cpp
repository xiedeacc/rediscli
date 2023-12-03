#include "redis.h"

#include <bits/stdc++.h>

#include "boost/algorithm/string/join.hpp"
#include "brpc/channel.h"
#include "brpc/redis.h"
#include "src/command/get.h"
#include "src/command/hmget.h"
#include "src/command/hset.h"
#include "src/command/set.h"
using namespace std;

int main(int argc, char **argv) {
  brpc::ChannelOptions options;
  options.protocol = brpc::PROTOCOL_REDIS;
  auto chan = make_shared<brpc::Channel>();
  if (0 != chan->Init("127.0.0.1:6379", &options)) {
    cerr << "failed to connect to 127.0.0.1:7000" << endl;
    return -1;
  }

  try {
    redis::Redis exec(chan, 30);
    auto ret = exec.Execute(redis::SET("mykey", 9878)).Get<string>();
    cout << "SET mykey 9878: " << ret << endl;
    auto ret2 = exec.Execute(redis::GET("mykey")).Get<int>();
    if (ret2 == 9878) {
      cout << "GET mykey: " << ret2 << endl;
    } else {
      cerr << "[FAILED] mykey has wrong value" << endl;
    }
  } catch (exception &e) {
    cerr << "catch exception: " << e.what() << endl;
  }

  try {
    redis::Redis exec(chan, 30);
    redis::SET cmd1("mykey", 9999);
    redis::GET cmd2("mykey");
    exec.Add(cmd1);
    exec.Add(cmd2);
    auto reply = exec.Execute();
    cout << cmd1.Text() << ": " << reply.Get<string>(0) << endl;
    cout << cmd2.Text() << ": " << reply.Get<int>(1) << endl;
  } catch (exception &e) {
    cerr << "catch exception: " << e.what() << endl;
  }

  try {
    redis::Redis exec(chan, 30);
    redis::SET cmd1("mykey", 1919);
    redis::GET cmd2("mykey");
    exec.Add(cmd1);
    exec.Add(cmd2);
    auto fut = exec.ExecuteAsync();
    auto reply = fut.Get();  // wait and get reply
    cout << cmd1.Text() << ": " << reply->Get<string>(0) << endl;
    cout << cmd2.Text() << ": " << reply->Get<int>(1) << endl;
    // cout << cmd2.Text() << ": " << reply->Get<int>(2) << endl; throw
    // invalid_argument
  } catch (exception &e) {
    cerr << "catch exception: " << e.what() << endl;
  }

  try {
    redis::Redis exec(chan, 30);
    unordered_map<string, int> name2score{{"peter", 88}, {"mary", 99}};
    redis::HSET cmd("mykey2", name2score);
    cout << cmd.Text() << ": " << exec.Execute(cmd).Get<int>() << endl;
    redis::HMGET cmd2("mykey2", {"mary", "peter", "haha"});
    cout << cmd2.Text() << endl;
    for (auto n : exec.Execute(cmd2).Get<vector<int>>()) {
      cout << "get score: " << n << endl;
    }
  } catch (exception &e) {
    cerr << "catch exception: " << e.what() << endl;
  }

  return 0;
}
