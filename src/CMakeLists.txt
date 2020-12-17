cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(bolo)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(GNU_FS_LIB stdc++fs)
endif()

# set include_directories
include_directories("include")

add_subdirectory(compress)
add_subdirectory(tar)
add_subdirectory(bolo)
add_subdirectory(app)
add_subdirectory(test)
