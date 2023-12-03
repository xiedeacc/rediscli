load("//tools:cpplint.bzl", "cpplint")
load("//tools:build.bzl", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "command",
    srcs = ["command.cpp"],
    hdrs = [
        "cluster.h",
        "command.h",
        "decrby.h",
        "del.h",
        "eval.h",
        "expire.h",
        "get.h",
        "hdel.h",
        "hget.h",
        "hgetall.h",
        "hincrby.h",
        "hkeys.h",
        "hmget.h",
        "hmset.h",
        "hset.h",
        "hvals.h",
        "incrby.h",
        "mget.h",
        "mset.h",
        "readonly.h",
        "set.h",
        "setex.h",
        "setnx.h",
        "smembers.h",
    ],
    deps = [
        "//src/detail",
        "@nlog//src:nlog",
    ],
)

cpplint()
