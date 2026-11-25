cmake_minimum_required(VERSION 3.21)
project(cppjavac)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE cppjavac_SOURCES CONFIGURE_DEPENDS
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
add_executable(cppjavac ${cppjavac_SOURCES}
        src/main/ast/parser/_Parser.cpp)
target_include_directories(cppjavac PRIVATE src)
set_target_properties(cppjavac PROPERTIES OUTPUT_NAME javac)

if(NOT WIN32)
    find_package(Iconv REQUIRED)
    target_link_libraries(cppjavac PRIVATE Iconv::Iconv)
endif()

if(MINGW)
    target_link_options(cppjavac PRIVATE -static -municode)
endif()
