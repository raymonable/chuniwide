cmake_minimum_required(VERSION 3.20)
project(chuniwide C)

set(CMAKE_C_STANDARD 23)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Chuniwide must be built with the `-A Win32` argument to build for 32-bit architecture." )
elseif(NOT MSVC)
    message(FATAL_ERROR "Chuniwide must be built with MSVC")
endif()

add_library(chuniwide OBJECT chuniwide.c)

add_library(chuniwide_25inch SHARED $<TARGET_OBJECTS:chuniwide>)
target_compile_definitions(chuniwide_25inch PUBLIC RESIZE_25_INCH)

add_library(chuniwide_27inch SHARED $<TARGET_OBJECTS:chuniwide>)
target_compile_definitions(chuniwide_25inch PUBLIC RESIZE_27_INCH)