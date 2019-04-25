cmake_minimum_required(VERSION 3.2)
project(huffman)

# add_subdirectory(fileio)
SET(SOURCE_FILES 
  ./main.c
  ./util.h
  ./huffbuild/huffbuild.h
  ./fileio/fileio.h
  ./fileio/fileio.c
  ./parallel/encode.h
  ./parallel/encode.c
  ./parallel/decode.h
  ./parallel/decode.c
  ./hashmap/hashmap.h
  ./hashmap/hashmap.c
)

SET(MPI_COMPILE_FLAGS "${MPI_COMPILE_FLAGS} -g -Wall -Wextra")

add_executable(huffman ${SOURCE_FILES})
target_link_libraries(huffman ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
  set_target_properties(huffman PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(huffman PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

install(TARGETS huffman DESTINATION ${PROJECT_INSTALL_BIN_DIR})