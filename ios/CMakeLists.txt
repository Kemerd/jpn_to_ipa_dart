# CMake configuration for iOS build of Japanese Phoneme Converter
# This file is used when building through Flutter's iOS build system

cmake_minimum_required(VERSION 3.14)
project(japanese_phoneme_converter)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    ../native/jpn_to_phoneme_ffi.cpp
)

# Create shared library
add_library(japanese_phoneme_converter SHARED ${SOURCES})

# Compiler flags
target_compile_options(japanese_phoneme_converter PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-exceptions
    -fno-rtti
    -O3
    -ffast-math
    -funroll-loops
)

# Define exports for FFI functions
target_compile_definitions(japanese_phoneme_converter PRIVATE
    FFI_EXPORT=__attribute__((visibility("default")))
)

# Link libraries
target_link_libraries(japanese_phoneme_converter
    c++
)

# Set properties
set_target_properties(japanese_phoneme_converter PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    FRAMEWORK TRUE
    MACOSX_FRAMEWORK_IDENTIFIER com.kemerd.japanese_phoneme_converter
    PUBLIC_HEADER "../native/jpn_to_phoneme_ffi.h"
)

# Install rules
install(TARGETS japanese_phoneme_converter
    LIBRARY DESTINATION lib
    FRAMEWORK DESTINATION Frameworks
    PUBLIC_HEADER DESTINATION include
)
