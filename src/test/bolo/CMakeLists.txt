add_executable(bolo_test test.cc)

target_link_libraries(bolo_test bolo tar ${GNU_FS_LIB})
