cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0048 NEW)
project(ObjectMutex, VERSION 2.0.0)

enable_testing()

message("[BUILD] ${BUILD_TARGET}")
include(${BUILD_TARGET}.cmake)

add_subdirectory(test)
