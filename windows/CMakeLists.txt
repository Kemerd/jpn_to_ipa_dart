# Flutter Windows FFI Plugin - Japanese Phoneme Converter

cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "japanese_phoneme_converter")
project(${PROJECT_NAME} LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific optimizations (MSVC)
if(MSVC)
    add_compile_options(/O2 /Ob2 /Oi /Ot /GL /utf-8)
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_link_options(/LTCG)
    endif()
endif()

# Build shared library (DLL)
add_library(${PROJECT_NAME} SHARED
    "../native/jpn_to_phoneme_ffi.cpp"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    POSITION_INDEPENDENT_CODE ON
)

target_compile_definitions(${PROJECT_NAME} PUBLIC DART_SHARED_LIB)

