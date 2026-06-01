cmake_minimum_required(VERSION 3.20)
project(chuniwide C)

set(CMAKE_C_STANDARD 23)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Chuniwide must be built with the `-A Win32` argument to build for 32-bit architecture." )
elseif(NOT MSVC)
    message(FATAL_ERROR "Chuniwide must be built with MSVC")
endif()

# Zydis

set(ZYDIS_FEATURE_ENCODER OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_SHARED_LIB OFF CACHE BOOL "" FORCE)

add_subdirectory(zydis)

# Add screen sizes

add_library(chuniwide_25inch SHARED chuniwide.c)
target_compile_definitions(chuniwide_25inch PUBLIC RESIZE_25_INCH)
target_link_libraries(chuniwide_25inch PUBLIC Zydis::Zydis)

add_library(chuniwide_27inch SHARED chuniwide.c)
target_compile_definitions(chuniwide_27inch PUBLIC RESIZE_27_INCH)
target_link_libraries(chuniwide_27inch PUBLIC Zydis::Zydis)