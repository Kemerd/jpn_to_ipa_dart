# Flutter Linux FFI Plugin - Japanese Phoneme Converter

cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "japanese_phoneme_converter")
project(${PROJECT_NAME} LANGUAGES CXX)

# C++17 required for optimal performance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Apply aggressive optimization flags only in Release mode
if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(-O3 -march=native -ffast-math -funroll-loops)
endif()

# Build shared library
add_library(${PROJECT_NAME} SHARED
    "../native/jpn_to_phoneme_ffi.cpp"
)

# Set library properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "../native/jpn_to_phoneme_ffi.h"
    OUTPUT_NAME "${PROJECT_NAME}"
)

# Install rules for Flutter
target_compile_definitions(${PROJECT_NAME} PUBLIC DART_SHARED_LIB)

