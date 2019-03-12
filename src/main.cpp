#include "main.h"
#include <serial.h>

int main(int argc, const char *argv[]) {

  std::cout << HEADER;

  // ensure the correct number of parameters are used.
  if (argc < 3) {
    std::cout << USAGE;
    return 1;
  }

  return 0;
}
