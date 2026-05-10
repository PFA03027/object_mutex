cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0048 NEW)
project(
    ObjectMutex
    #
    DESCRIPTION "A wrapper class that exclusively controls access to class instances"
    LANGUAGES CXX
    VERSION 2.0.1
)

enable_testing()

message("[BUILD] ${BUILD_CONFIG}")
include(${BUILD_CONFIG}.cmake)

add_subdirectory(object_mutex)
add_subdirectory(test EXCLUDE_FROM_ALL)
