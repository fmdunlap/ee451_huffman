//
// Created by Konstantin Gredeskoul on 5/14/17.
//

#ifndef CMAKE_SERIAL_H
#define CMAKE_SERIAL_H

static const char *const MISSING_SOURCE_FILE_MESSAGE = "Your source file is missing!";
static const char *const MISSING_DEST_FILE_MESSAGE = "Your destination file is missing!";

#include <iostream>
#include <stdexcept>

using namespace std;

class HuffmanSerial {
public:
  explicit HuffmanSerial() {
  }

  ~HuffmanSerial() {
  };

  int run();
};

#endif //CMAKE_SERIAL_H
