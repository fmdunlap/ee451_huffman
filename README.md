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
 * RUBY ```bash sudo apt-get install ruby``` (Already installed on the USC HPC.) 
 * Mpich ```bash sudo apt-get install mpich``` (Already installed on the USC HPC.) 
 * Realpath ```bash sudo apt-get install realpath``` (Already installed on the USC HPC.) 

### Building The Project

####  Building Using the Script

I built a handy BASH script that you can use to build the project. It's a bit complicated, so I highly recommend using this.

```bash
./build.sh
```

#### Building Manually

Plz just don't build manually.

### Running the Project

Similar to the build script, I put together a handy-dandy run script. This formats your command to be usable by the C program, and keeps things (somewhat) portable. Plus, I really like how pretty lib-bash makes things

Options:
 * -n/--numprocs - Sets number of processes.
 * -t/--type - Sets the program to run in (s)erial or (p)arallel.
 * -o/--output - Sets the location of the output file.
 * -s/--srun - Whether to use srun or not. Setting this means you are using srun.

```bash
./run.sh [-n/--numprocs] <num_procs> [-t/--type] <s or p> [-o/--output] <./out/file/location> [-s/--srun] ./fileToCompress
```


## File Locations

 * `src/*` — C++ code that ultimately compiles into a library
 * `bin/`, `lib`, `include` are all empty directories, until the `make install` install the project artifacts there.

### Acknowledgements

This project is a derivative of the [CMake C++ Project Template](https://github.com/kigster/cmake-project-template).
