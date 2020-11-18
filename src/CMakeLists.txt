cmake_minimum_required(VERSION 3.10)

# set the project name
project(bolo)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# add the executable
add_executable(bolo main.cc)
target_link_libraries(bolo Qt5::Widgets)
