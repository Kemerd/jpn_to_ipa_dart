/// Blazing fast Japanese to IPA phoneme converter using native FFI.
///
/// This library provides high-performance conversion of Japanese text to
/// International Phonetic Alphabet (IPA) representation using a native
/// C++ library via FFI.
///
/// ## Features
/// - **Ultra-fast**: Microsecond-level conversion times
/// - **Cross-platform**: Works on Windows, Linux, macOS, Android, iOS
/// - **Simple API**: Just load and convert
/// - **Performance metrics**: Get conversion time for every operation
///
/// ## Example
/// ```dart
/// import 'package:japanese_phoneme_converter/japanese_phoneme_converter.dart';
///
/// void main() {
///   // Initialize converter
///   final converter = JapanesePhonemeConverter();
///   
///   if (!converter.init('path/to/ja_phonemes.json')) {
///     print('Failed to initialize');
///     return;
///   }
///   
///   // Convert text
///   final result = converter.convert('こんにちは');
///   if (result != null) {
///     print('Phonemes: ${result.phonemes}');
///     print('Time: ${result.processingTimeMicroseconds}μs');
///   }
///   
///   // Clean up
///   converter.dispose();
/// }
/// ```
library japanese_phoneme_converter;

export 'src/japanese_phoneme_converter.dart';
export 'src/conversion_result.dart';
export 'src/phoneme_exception.dart';

