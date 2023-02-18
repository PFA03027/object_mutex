project(ObjectMutexTest)
cmake_minimum_required(VERSION 3.16)

find_package(Git QUIET)

# Gitがインストールされており、トップディレクトリに.gitディレクトリがある
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/../.git")
    enable_testing()
    option(GOOGLETEST "Check Google Test during build" ON)
    # -DGOOGLETEST=onオプションを付けて実行したらsubmoduleを最新版にする
    if(GOOGLETEST)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/googletest/CMakeLists.txt")
message(FATAL_ERROR "The submodules were not downloaded! GOOGLETEST was turned off or failed. Please update submodules and try again.")
# mkdir -p googletest
# git submodule add https://github.com/google/googletest.git googletest
# cd googletest
# git checkout release-1.12.1
# cd ..
# git add test/lib/googletest
# git commit -m "Add submodule googletest for testing"
endif()


add_subdirectory(googletest)


# file(GLOB SOURCES src/*.cpp )
set(SOURCES test.cpp)

add_executable(test_object_mutex ${SOURCES})
target_include_directories(test_object_mutex PRIVATE ../inc)
target_link_libraries(test_object_mutex gtest gtest_main pthread)

add_test(NAME test_object_mutex COMMAND $<TARGET_FILE:test_object_mutex>)
