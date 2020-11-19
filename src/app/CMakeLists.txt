# qt5
find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(app main.cc)

target_link_libraries(app PUBLIC Qt5::Widgets compress tar)
