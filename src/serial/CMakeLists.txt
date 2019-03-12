cmake_minimum_required(VERSION 3.2)
project(serial C CXX)

set(SOURCE_FILES
    serial.h
    serial.cpp
)

add_library(serial SHARED STATIC ${SOURCE_FILES})

install(TARGETS serial DESTINATION ${HUFFMAN_INSTALL_LIB_DIR})
install(FILES serial.h DESTINATION ${HUFFMAN_INSTALL_INCLUDE_DIR})
