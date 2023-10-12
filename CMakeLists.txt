cmake_minimum_required(VERSION 3.16)

project(MultiKeyMap LANGUAGES CXX)

option(mkm_BuildTests "Build MultiKeyMap tests." ON)

add_library(MultiKeyMap INTERFACE)
add_library(MultiKeyMap::MultiKeyMap ALIAS MultiKeyMap)

target_include_directories(MultiKeyMap INTERFACE inc)
target_compile_features(MultiKeyMap INTERFACE cxx_std_17)

# Unit tests
if(mkm_BuildTests)
    set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${CMAKE_PREFIX_PATH}
    )

    add_subdirectory(tests)
endif(mkm_BuildTests)

