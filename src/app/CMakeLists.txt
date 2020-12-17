# qt5
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(app main.cc)

target_link_libraries(app PUBLIC Qt5::Widgets 
                      bolo tar compress
                      ${GNU_FS_LIB})
