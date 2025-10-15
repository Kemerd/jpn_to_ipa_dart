# Flutter Android FFI Plugin - Japanese Phoneme Converter

cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "japanese_phoneme_converter")
project(${PROJECT_NAME} LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Apply Android-specific optimizations only in Release mode
if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(-O3 -ffast-math -funroll-loops)
endif()

# Build shared library for Android
add_library(${PROJECT_NAME} SHARED
    "../native/jpn_to_phoneme_ffi.cpp"
)

# Link against Android log library
find_library(log-lib log)
target_link_libraries(${PROJECT_NAME} ${log-lib})

target_compile_definitions(${PROJECT_NAME} PUBLIC DART_SHARED_LIB)

# ============================================================================
# Installation and bundling for Flutter
# ============================================================================

# The library is automatically bundled by Flutter's Android build system
# No additional configuration needed for Android FFI plugins

