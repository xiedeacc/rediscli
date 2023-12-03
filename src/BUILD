load("//tools:cpplint.bzl", "cpplint")
load("//tools:build.bzl", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "callback",
    srcs = [
        "reply.cpp",
    ],
    hdrs = [
        "callback.h",
        "reply.h",
    ],
    deps = [
        "//src/detail:get_reply",
        "@boost//:lexical_cast",
        "@brpc",
        "@nlog//src:nlog",
    ],
)

cc_library(
    name = "rediscli",
    srcs = [
        "cluster_map.cpp",
        "future.cpp",
        "redis.cpp",
        "redis_cluster.cpp",
        "reply.cpp",
    ],
    hdrs = [
        "cluster_map.h",
        "future.h",
        "redis.h",
        "redis_cluster.h",
        "reply.h",
    ],
    deps = [
        ":callback",
        "//src/command",
        "//src/detail",
    ],
)

#cc_test(
#name = "redis_test",
#srcs = [
#"redis_test.cpp",
#],
#data = [
#],
#deps = [
#":rediscli",
#"@com_google_googletest//:gtest",
#],
#)

#cc_test(
#name = "redis_cluster_test",
#srcs = [
#"redis_cluster_test.cpp",
#],
#data = [
#],
#deps = [
#":rediscli",
#"@com_google_googletest//:gtest",
#],
#)

cpplint()
