cmake_minimum_required(VERSION 3.16)

project(GenericTrieCPP)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_PREFIX_PATH}
)

# add_compile_options(-Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-variable -Wno-unused-label -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-function -Wno-error=deprecated -Wno-error=deprecated-declarations -g)
# add_executable(unit_tests tests/main.cpp)

# target_include_directories(unit_tests PRIVATE inc)

add_subdirectory(tests)

