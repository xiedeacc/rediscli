#include <bits/stdc++.h>

#include "cluster_map.h"
#include "command/get.h"
#include "command/hmget.h"
#include "command/hset.h"
#include "command/set.h"
#include "redis_cluster.h"
using namespace std;

const char *config = "my_cluster\n127.0.0.1:7000\n127.0.0.1:7001";

int main(int argc, char **argv) {
  redis::SlotMap slotmap;
  if (0 != slotmap.Load(config)) {
    return -1;
  }

  try {
    redis::RedisCluster cluster(&slotmap, 30);
    redis::GET cmd1("mykey1");
    redis::GET cmd2("mykey2");
    cluster.Add(cmd1);
    cluster.Add(cmd2);
    auto reply = cluster.Execute();
    cout << cmd1.Text() << ": " << reply.Get<string>(0) << endl;
    cout << cmd2.Text() << ": " << reply.Get<string>(1) << endl;
  } catch (exception &e) {
    cerr << "catch exception: " << e.what() << endl;
  }

  try {
    redis::RedisCluster cluster(&slotmap, 30);
    redis::GET cmd1("mykey1");
    redis::GET cmd2("mykey2");
    cluster.Add(cmd1);
    cluster.Add(cmd2);
    auto fut = cluster.ExecuteAsync();
    auto reply = fut.Get();
    cout << cmd1.Text() << ": " << reply->Get<string>(0) << endl;
    cout << cmd2.Text() << ": " << reply->Get<string>(1) << endl;
  } catch (exception &e) {
    cerr << "catch exception: " << e.what() << endl;
  }

  try {
    redis::RedisCluster cluster(&slotmap, 30);
    redis::SET cmd1("mykey1", 1);
    redis::SET cmd2("mykey2", 2);
    cluster.Add(cmd1);
    cluster.Add(cmd2);
    auto fut = cluster.ExecuteAsync();
    auto reply = fut.Get();
    cout << cmd1.Text() << ": " << reply->Get<string>(0) << endl;
    cout << cmd2.Text() << ": " << reply->Get<string>(1) << endl;
  } catch (exception &e) {
    cerr << "catch exception: " << e.what() << endl;
  }

  try {
    redis::RedisCluster cluster(&slotmap, 30);
    redis::SET cmd1("mykey1", 33);
    redis::SET cmd2("mykey1", 88);
    cluster.Add(cmd1);
    cluster.Add(cmd2);
    auto fut = cluster.ExecuteAsync();
    auto reply = fut.Get();
    cout << cmd1.Text() << ": " << reply->Get<string>(0) << endl;
    cout << cmd2.Text() << ": " << reply->Get<string>(1) << endl;
  } catch (exception &e) {
    cerr << "catch exception: " << e.what() << endl;
  }
  return 0;
}