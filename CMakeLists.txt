cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bins)

set(SOURCES_FOLDER "src")
set(TESTS_FOLDER "tests")

project(simple_message_queue
        VERSION 1.0
        LANGUAGES CXX)

include_directories(SOURCES_FOLDER)

file(GLOB ALL_SOURCES "${SOURCES_FOLDER}/*.cpp")
file(GLOB ALL_TESTS "${TESTS_FOLDER}/*.cpp")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(tests "${ALL_SOURCES}" "${ALL_TESTS}")
target_link_libraries(tests gmock gtest_main)
target_include_directories(tests PRIVATE ${SOURCES_FOLDER})
gtest_discover_tests(tests)

