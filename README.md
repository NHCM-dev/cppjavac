# cppjavac

A simple Java compiler written in C++ trying to achieve the functionality of original javac while enabling
some interesting compiler extensions, improve performance and better user experience.  
While most of the feature are still not implemented, further development is still on schedule.

## Build

### Requirements

- CMake 3.21 or newer
- Ninja
- A C++23 toolchain.

### Release build

Clone repository.

```shell
git clone https://github.com/NHCM-dev/cppjavac cppjavac
cd cppjavac
```

run these commands in your project directory.

```shell
cmake --preset release
cmake --build --preset release
```

The output will be at `./cmake-build-release/javac.exe`.
