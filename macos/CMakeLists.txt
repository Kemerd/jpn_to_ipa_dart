# Flutter macOS FFI Plugin - Japanese Phoneme Converter

cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "japanese_phoneme_converter")
project(${PROJECT_NAME} LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Apply macOS-specific optimizations only in Release mode
if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(-O3 -march=native -ffast-math)
endif()

# Build shared library
add_library(${PROJECT_NAME} SHARED
    "../native/jpn_to_phoneme_ffi.cpp"
)

# macOS framework settings
set_target_properties(${PROJECT_NAME} PROPERTIES
    FRAMEWORK TRUE
    OUTPUT_NAME "${PROJECT_NAME}"
    MACOSX_FRAMEWORK_IDENTIFIER "com.kemerd.${PROJECT_NAME}"
)

target_compile_definitions(${PROJECT_NAME} PUBLIC DART_SHARED_LIB)

# ============================================================================
# Installation and bundling for Flutter
# ============================================================================

# Install the framework to the bundled libraries location
# This ensures Flutter copies it to the app bundle at build time
set(japanese_phoneme_converter_bundled_libraries
  $<TARGET_FILE:${PROJECT_NAME}>
  PARENT_SCOPE
)

