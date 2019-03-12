cmake_minimum_required(VERSION 3.2)
project(huffman)

add_subdirectory(serial)
set(SOURCE_FILES main.cpp)

add_executable(huffman ${SOURCE_FILES})
target_link_libraries(huffman serial)
install(TARGETS huffman DESTINATION ${HUFFMAN_INSTALL_BIN_DIR})
