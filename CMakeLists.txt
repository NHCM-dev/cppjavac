cmake_minimum_required(VERSION 4.4)
project(JavaCompiler)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE JavaCompiler_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hh"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inc")
add_executable(JavaCompiler ${JavaCompiler_SOURCES}
        src/main/Compiler.cpp
        src/main/Compiler.h)
target_include_directories(JavaCompiler PRIVATE src)
set_target_properties(JavaCompiler PROPERTIES OUTPUT_NAME javac)

if(MINGW)
    target_link_options(JavaCompiler PRIVATE -static)
endif()
