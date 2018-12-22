# refalloc
The `refalloc` library provides a simple API for allocating memory with
reference counting. Multi-threading is supported via atomic operations
when available. Pointer cycles (like doubly-linked lists) are not
supported.

## Goals
This project aims to provide easy-to-use basic reference counting for
memory blocks allocated in C. In addition:

- The reference counting should be robust against race conditions in
  multi-threaded applications. This is currently implemented using
  some atomic operations.

- The interface should be simple and orthogonal.

- The source should not use more language features or libraries
  than necessary, and should not be larger (in lines of code)
  than needed.

## Build

This project uses CMake for building. Developers can obtain CMake from
the following URL:
[https://cmake.org/download/](https://cmake.org/download/)

To use CMake with this project, first make a directory to hold the build
results. Then run CMake in the directory with a path to the source code.
On UNIX, the commands would look like the following:
```
mkdir build
cd build
cmake ../refalloc
```

Running CMake should create a build project, which then can be processed
using other tools. Usually, the tool would be Makefile or a IDE project.
For Makefiles, use the following command to build the project:
```
make
```
For IDE projects, the IDE must be installed and ready to use. Open the
project within the IDE.

Since this project's source only holds two files, developers could also
use these files independently from CMake.

## License
This project uses the Unlicense, which makes the source effectively
public domain. Go to [http://unlicense.org/](http://unlicense.org/)
to learn more about the Unlicense.

Contributions to this project should likewise be provided under a
public domain dedication.
