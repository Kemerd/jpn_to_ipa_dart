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
  s.source_files     = 'Classes/**/*'
  s.public_header_files = 'Classes/**/*.h'
  
  # Platform configuration
  s.ios.deployment_target = '11.0'
  
  # Build C++ source
  s.vendored_frameworks = 'Frameworks/JapanesePhonemeConverter.framework'
  s.preserve_paths = '../native/jpn_to_phoneme_ffi.cpp'
  
  # C++ settings
  s.xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'CLANG_CXX_LIBRARY' => 'libc++',
    'OTHER_CFLAGS' => '-O3 -ffast-math'
  }
  
  # Flutter dependency
  s.dependency 'Flutter'
  
  # Platform setup
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES' }
  s.swift_version = '5.0'
end

