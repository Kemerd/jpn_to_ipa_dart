# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(japanese_phoneme_converter_library VERSION 0.0.1 LANGUAGES CXX)

add_library(japanese_phoneme_converter SHARED
  "jpn_to_phoneme_ffi.cpp"
)

# Set C++17 standard (required for the phoneme converter)
set_target_properties(japanese_phoneme_converter PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  OUTPUT_NAME "japanese_phoneme_converter"
)

target_compile_definitions(japanese_phoneme_converter PUBLIC DART_SHARED_LIB)

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(japanese_phoneme_converter PRIVATE "-Wl,-z,max-page-size=16384")
endif()
