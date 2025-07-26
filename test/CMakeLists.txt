cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0048 NEW)
project(ObjectMutexTest, VERSION 2.0.0)


include(FetchContent)
FetchContent_Declare(
  googletest
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
)
#   URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# file(GLOB SOURCES src/*.cpp )
set(SOURCES test.cpp)

add_executable(test_object_mutex ${SOURCES})
target_include_directories( test_object_mutex  PRIVATE ../inc )
target_compile_options(test_object_mutex PUBLIC  -Wall -Wconversion -Wsign-conversion -Werror)
target_link_libraries(test_object_mutex gtest gtest_main )
# add_dependencies(build-test test_object_mutex)
add_test(NAME test_object_mutex COMMAND $<TARGET_FILE:test_object_mutex>)

