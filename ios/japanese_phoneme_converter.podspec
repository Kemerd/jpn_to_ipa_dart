#
# Japanese Phoneme Converter - iOS Flutter Plugin
#
Pod::Spec.new do |s|
  s.name             = 'japanese_phoneme_converter'
  s.version          = '1.0.0'
  s.summary          = 'Blazing fast Japanese to IPA phoneme converter using native FFI'
  s.description      = <<-DESC
High-performance Japanese text to IPA phoneme conversion using optimized C++ implementation.
                       DESC
  s.homepage         = 'https://github.com/Kemerd/japanese-phoneme-converter'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Kemerd' => 'your.email@example.com' }
  
  # Source location
  s.source           = { :path => '.' }
  
  # Include both Objective-C plugin files and C++ FFI source
  s.source_files = ['Classes/**/*', '../native/jpn_to_phoneme_ffi.cpp']
  s.public_header_files = 'Classes/**/*.h'
  s.preserve_paths = '../native/jpn_to_phoneme_ffi.cpp'
  
  # Platform configuration
  s.ios.deployment_target = '11.0'
  
  # C++ settings - apply optimization flags only in Release builds
  # This prevents conflicts with Debug runtime checks
  s.xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'CLANG_CXX_LIBRARY' => 'libc++',
    'OTHER_CFLAGS[config=Release]' => '-O3 -ffast-math -funroll-loops',
    'OTHER_CFLAGS[config=Debug]' => '',
    'GCC_OPTIMIZATION_LEVEL[config=Release]' => '3',
    'GCC_OPTIMIZATION_LEVEL[config=Debug]' => '0'
  }
  
  # Compile C++ files as C++
  s.compiler_flags = '-x objective-c++'
  
  # Flutter dependency
  s.dependency 'Flutter'
  
  # Platform setup
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES' }
  s.swift_version = '5.0'
end

