cmake_minimum_required(VERSION 3.16)

find_package(gtest REQUIRED)

set(TEST_SOURCES
    MultiKeyMapTests.cpp
)

add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE gtest gtest_main gmock)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/inc)
target_compile_options(unit_tests PRIVATE -Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-variable -Wno-unused-label -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-function -Wno-error=deprecated -Wno-error=deprecated-declarations -g)

