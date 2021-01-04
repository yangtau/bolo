add_library(bolo STATIC bolo.cc)
target_link_libraries(bolo tar compress crypto)
