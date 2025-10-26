#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint japanese_phoneme_converter.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'japanese_phoneme_converter'
  s.version          = '2.3.0'
  s.summary          = 'A new Flutter FFI plugin project.'
  s.description      = <<-DESC
A new Flutter FFI plugin project.
                       DESC
  s.homepage         = 'http://example.com'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Your Company' => 'email@example.com' }

  # This will ensure the source files in Classes/ are included in the native
  # builds of apps using this FFI plugin. Podspec does not support relative
  # paths, so Classes contains a forwarder C++ file that relatively imports
  # `../src/*` so that the C++ sources can be shared among all target platforms.
  s.source           = { :path => '.' }
  s.source_files = 'Classes/**/*'
  s.dependency 'Flutter'
  s.platform = :ios, '13.0'

  # C++17 is required for the phoneme converter
  # Flutter.framework does not contain a i386 slice.
  s.pod_target_xcconfig = { 
    'DEFINES_MODULE' => 'YES', 
    'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386',
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'CLANG_CXX_LIBRARY' => 'libc++',
    # Prevent symbol stripping on iOS - CRITICAL for FFI
    'STRIP_STYLE' => 'non-global',
    'DEAD_CODE_STRIPPING' => 'NO',
    # Ensure all symbols are visible
    'GCC_SYMBOLS_PRIVATE_EXTERN' => 'NO',
    # Force symbols to be included
    'OTHER_LDFLAGS' => '-all_load'
  }
  s.swift_version = '5.0'
  
  # Explicitly link C++ standard library
  s.library = 'c++'
end
