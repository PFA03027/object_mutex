cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0048 NEW)
project(
    ObjectMutex
    #
    DESCRIPTION "A wrapper class that exclusively controls access to class instances"
    LANGUAGES CXX
    VERSION 1.1.2
)

enable_testing()

message("[BUILD] ${BUILD_CONFIG}")
include(${BUILD_CONFIG}.cmake)

add_subdirectory(object_mutex)
add_subdirectory(test EXCLUDE_FROM_ALL)
