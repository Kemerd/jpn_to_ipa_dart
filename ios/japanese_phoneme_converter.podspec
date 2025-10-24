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
  
  # Include plugin files and C++ FFI source
  s.source_files = 'Classes/**/*.{h,m,mm}', '../native/jpn_to_phoneme_ffi.cpp'
  s.public_header_files = 'Classes/**/*.h'
  s.preserve_paths = ['Classes/**/*', '../native/jpn_to_phoneme_ffi.cpp']
  
  # Add the assets directory to resources
  s.resources = ['../assets/*']
  
  # Platform configuration
  s.ios.deployment_target = '11.0'
  
  # C++ settings - apply optimization flags only in Release builds
  # This prevents conflicts with Debug runtime checks
  s.xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'CLANG_CXX_LIBRARY' => 'libc++',
    'OTHER_CPLUSPLUSFLAGS[config=Release]' => '-O3 -ffast-math -funroll-loops -fvisibility=hidden',
    'OTHER_CPLUSPLUSFLAGS[config=Debug]' => '-fvisibility=hidden',
    'GCC_OPTIMIZATION_LEVEL[config=Release]' => '3',
    'GCC_OPTIMIZATION_LEVEL[config=Debug]' => '0',
    'GCC_SYMBOLS_PRIVATE_EXTERN' => 'NO',
    'GCC_ENABLE_CPP_EXCEPTIONS' => 'YES',
    'GCC_ENABLE_CPP_RTTI' => 'YES',
    'ENABLE_BITCODE' => 'NO'
  }
  
  # Compile C++ files as C++ with proper flags
  s.compiler_flags = '-fno-objc-arc -stdlib=libc++ -std=c++17'
  
  # Flutter dependency
  s.dependency 'Flutter'
  
  # Platform setup
  s.pod_target_xcconfig = {
    'DEFINES_MODULE' => 'YES',
    'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386',
    'VALID_ARCHS' => 'x86_64 arm64',
    'OTHER_LDFLAGS' => '-ObjC -all_load'
  }
  s.swift_version = '5.0'
  
  # Ensure FFI symbols are exported
  s.libraries = 'c++'
end

