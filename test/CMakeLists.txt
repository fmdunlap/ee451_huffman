cmake_minimum_required(VERSION 3.2)
project(huffman_tests)

add_subdirectory(lib/googletest)

include_directories(${SERIAL_HEADERS_DIR})
include_directories(lib/googletest/googletest/include)

set(SOURCE_FILES main.cpp src/serial_tests.cpp)

add_executable(serial_tests ${SOURCE_FILES})
target_link_libraries(serial_tests serial gtest)
install(TARGETS serial_tests DESTINATION bin)

