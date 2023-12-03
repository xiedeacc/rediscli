load("//tools:cpplint.bzl", "cpplint")
load("//tools:build.bzl", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "get_reply",
    srcs = [
    ],
    hdrs = [
        "get_reply.h",
    ],
    deps = [
        "@nlog//src:nlog",
    ],
)

cc_library(
    name = "detail",
    srcs = [
        "async_data.cpp",
        "crc16.cpp",
        "flags.cpp",
        "hashslot.cpp",
        "passwd_map.cpp",
        "redis_channel_cache.cpp",
    ],
    hdrs = [
        "add_components.h",
        "async_data.h",
        "command_type_traits.h",
        "flags.h",
        "passwd_map.h",
        "redis_channel_cache.h",
    ],
    deps = [
        "//src:callback",
        "@boost//:thread",
        "@brpc",
        "@nlog//src:nlog",
    ],
)

cpplint()
