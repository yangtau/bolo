cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(bolo)

# set include_directories
include_directories("include")

add_subdirectory(compress)
add_subdirectory(tar)
add_subdirectory(app)
