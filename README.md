# EE451 Huffman Encoding

### Overview

Hi, y'all! Here's the bones of our soon-to-be encryption project.

Take a peak at the instructions below on how to build. I've already put some of the basics in, but not much.

Source: I just modified an excellent template (https://github.com/kigster/cmake-project-template)

## Usage

### Prerequisites

You will need:

 * A modern C/C++ compiler
 * CMake 3.1+ installed (on a Mac, run `brew install cmake`)
 * If you need a text editor, I HIGHLY recommend [VSCode](https://code.visualstudio.com/).

### Building The Project

#### Project Structure

There are three empty folders: `lib`, `bin`, and `include`. Those are populated by `make install`.

The rest should be obvious: `src` is the sources, and `test` is where we put our unit tests.

Now we can build this project, and below we show three separate ways to do so.

#### Building Manually

```bash
$ rm -rf build/manual && mkdir build/manual
$ cd build/manual
$ cmake ../..
$ make && make install
$ cd ../..

# Run the tests:
$ bin/divider_tests 

# Run the binary:
$ bin/divider 234 5431
```

####  Building Using the Script

There is a handy BASH script (used by the Travis CI) that you can run locally. It builds the project, and runs all the tests

```bash
./run.sh
```

## File Locations

 * `src/*` — C++ code that ultimately compiles into a library
 * `test/lib` — C++ libraries used for tests (eg, Google Test)
 * `test/src` — C++ test suite
 * `bin/`, `lib`, `include` are all empty directories, until the `make install` install the project artifacts there.

Tests:

 * A `test` folder with the automated tests and fixtures that mimics the directory structure of `src`.
 * For every C++ file in `src/A/B/<name>.cpp` there is a corresponding test file `test/A/B/<name>_test.cpp`
 * Tests compile into a single binary `test/bin/runner` that is run on a command line to run the tests.
 * `test/lib` folder with a git submodule in `test/lib/googletest`, and possibly other libraries.

### Acknowledgements

This project is a derivative of the [CMake C++ Project Template](https://github.com/kigster/cmake-project-template).
