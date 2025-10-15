import 'dart:convert'; // For utf8.decode
import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:ffi/ffi.dart';

import 'conversion_result.dart';
import 'phoneme_exception.dart';

// ============================================================================
// FFI Type Definitions
// ============================================================================

/// Native function: int jpn_phoneme_init(const char* json_file_path)
typedef _InitNative = ffi.Int32 Function(ffi.Pointer<Utf8> jsonFilePath);
typedef _InitDart = int Function(ffi.Pointer<Utf8> jsonFilePath);

/// Native function: int jpn_phoneme_convert(...)
typedef _ConvertNative = ffi.Int32 Function(
  ffi.Pointer<Utf8> japaneseText,
  ffi.Pointer<ffi.Uint8> outputBuffer,
  ffi.Int32 bufferSize,
  ffi.Pointer<ffi.Int64> processingTimeUs,
);
typedef _ConvertDart = int Function(
  ffi.Pointer<Utf8> japaneseText,
  ffi.Pointer<ffi.Uint8> outputBuffer,
  int bufferSize,
  ffi.Pointer<ffi.Int64> processingTimeUs,
);

/// Native function: const char* jpn_phoneme_get_error()
typedef _GetErrorNative = ffi.Pointer<Utf8> Function();
typedef _GetErrorDart = ffi.Pointer<Utf8> Function();

/// Native function: int jpn_phoneme_get_entry_count()
typedef _GetEntryCountNative = ffi.Int32 Function();
typedef _GetEntryCountDart = int Function();

/// Native function: void jpn_phoneme_cleanup()
typedef _CleanupNative = ffi.Void Function();
typedef _CleanupDart = void Function();

/// Native function: const char* jpn_phoneme_version()
typedef _VersionNative = ffi.Pointer<Utf8> Function();
typedef _VersionDart = ffi.Pointer<Utf8> Function();

/// Native function: int jpn_phoneme_init_word_dict(const char* word_file_path)
typedef _InitWordDictNative = ffi.Int32 Function(ffi.Pointer<Utf8> wordFilePath);
typedef _InitWordDictDart = int Function(ffi.Pointer<Utf8> wordFilePath);

/// Native function: void jpn_phoneme_set_use_segmentation(bool enabled)
typedef _SetUseSegmentationNative = ffi.Void Function(ffi.Bool enabled);
typedef _SetUseSegmentationDart = void Function(bool enabled);

/// Native function: bool jpn_phoneme_get_use_segmentation()
typedef _GetUseSegmentationNative = ffi.Bool Function();
typedef _GetUseSegmentationDart = bool Function();

/// Native function: int jpn_phoneme_get_word_count()
typedef _GetWordCountNative = ffi.Int32 Function();
typedef _GetWordCountDart = int Function();

// ============================================================================
// Japanese Phoneme Converter - Main Class
// ============================================================================

/// High-performance Japanese to IPA phoneme converter using native FFI.
///
/// This class provides blazing fast conversion of Japanese text to
/// International Phonetic Alphabet representation using a native C++ library.
///
/// ## Usage
///
/// ```dart
/// // Create converter instance
/// final converter = JapanesePhonemeConverter();
///
/// // Initialize with dictionary file
/// if (!converter.init('path/to/ja_phonemes.json')) {
///   print('Initialization failed: ${converter.lastError}');
///   return;
/// }
///
/// // Convert Japanese text
/// final result = converter.convert('こんにちは');
/// print('Phonemes: ${result?.phonemes}');
/// print('Time: ${result?.processingTimeMicroseconds}μs');
///
/// // Clean up when done
/// converter.dispose();
/// ```
///
/// ## Thread Safety
///
/// After initialization, the converter is thread-safe for conversion operations.
/// However, initialization and cleanup should only be called from a single thread.
class JapanesePhonemeConverter {
  ffi.DynamicLibrary? _lib;
  _InitDart? _init;
  _ConvertDart? _convert;
  _GetErrorDart? _getError;
  _GetEntryCountDart? _getEntryCount;
  _CleanupDart? _cleanup;
  _VersionDart? _version;
  _InitWordDictDart? _initWordDict;
  _SetUseSegmentationDart? _setUseSegmentation;
  _GetUseSegmentationDart? _getUseSegmentation;
  _GetWordCountDart? _getWordCount;

  bool _isInitialized = false;
  bool _isDisposed = false;

  /// Default buffer size for conversion output (4KB)
  static const int defaultBufferSize = 4096;

  /// Creates a new phoneme converter instance.
  ///
  /// The native library is loaded automatically based on the current platform.
  /// Throws [PhonemeException] if the library cannot be loaded.
  JapanesePhonemeConverter({String? libraryPath}) {
    try {
      _lib = _loadLibrary(libraryPath);
      _bindFunctions();
    } catch (e) {
      throw PhonemeException('Failed to load native library: $e');
    }
  }

  /// Load the native library based on platform
  ffi.DynamicLibrary _loadLibrary(String? customPath) {
    if (customPath != null) {
      return ffi.DynamicLibrary.open(customPath);
    }

    // Load as Flutter FFI plugin (auto-bundled with app)
    const libName = 'japanese_phoneme_converter';
    
    if (Platform.isAndroid || Platform.isLinux) {
      return ffi.DynamicLibrary.open('lib$libName.so');
    } else if (Platform.isIOS || Platform.isMacOS) {
      return ffi.DynamicLibrary.process();
    } else if (Platform.isWindows) {
      return ffi.DynamicLibrary.open('$libName.dll');
    } else {
      throw PhonemeException('Unsupported platform: ${Platform.operatingSystem}');
    }
  }

  /// Bind native functions to Dart functions
  void _bindFunctions() {
    final lib = _lib!;

    _init = lib
        .lookup<ffi.NativeFunction<_InitNative>>('jpn_phoneme_init')
        .asFunction();
    _convert = lib
        .lookup<ffi.NativeFunction<_ConvertNative>>('jpn_phoneme_convert')
        .asFunction();
    _getError = lib
        .lookup<ffi.NativeFunction<_GetErrorNative>>('jpn_phoneme_get_error')
        .asFunction();
    _getEntryCount = lib
        .lookup<ffi.NativeFunction<_GetEntryCountNative>>('jpn_phoneme_get_entry_count')
        .asFunction();
    _cleanup = lib
        .lookup<ffi.NativeFunction<_CleanupNative>>('jpn_phoneme_cleanup')
        .asFunction();
    _version = lib
        .lookup<ffi.NativeFunction<_VersionNative>>('jpn_phoneme_version')
        .asFunction();
    _initWordDict = lib
        .lookup<ffi.NativeFunction<_InitWordDictNative>>('jpn_phoneme_init_word_dict')
        .asFunction();
    _setUseSegmentation = lib
        .lookup<ffi.NativeFunction<_SetUseSegmentationNative>>('jpn_phoneme_set_use_segmentation')
        .asFunction();
    _getUseSegmentation = lib
        .lookup<ffi.NativeFunction<_GetUseSegmentationNative>>('jpn_phoneme_get_use_segmentation')
        .asFunction();
    _getWordCount = lib
        .lookup<ffi.NativeFunction<_GetWordCountNative>>('jpn_phoneme_get_word_count')
        .asFunction();
  }

  /// Initialize the converter with a phoneme dictionary JSON file.
  ///
  /// This must be called before any conversion operations.
  /// Returns `true` on success, `false` on failure.
  ///
  /// Example:
  /// ```dart
  /// if (!converter.init('assets/ja_phonemes.json')) {
  ///   print('Failed: ${converter.lastError}');
  /// }
  /// ```
  bool init(String jsonFilePath) {
    _checkNotDisposed();

    final pathPtr = jsonFilePath.toNativeUtf8();
    try {
      final result = _init!(pathPtr);
      _isInitialized = result == 1;
      return _isInitialized;
    } finally {
      malloc.free(pathPtr);
    }
  }

  /// Convert Japanese text to IPA phonemes.
  ///
  /// Returns a [ConversionResult] containing the phonemes and processing time,
  /// or `null` if conversion fails.
  ///
  /// Example:
  /// ```dart
  /// final result = converter.convert('日本語');
  /// if (result != null) {
  ///   print('${result.phonemes} (${result.processingTimeMicroseconds}μs)');
  /// }
  /// ```
  ConversionResult? convert(String japaneseText, {int bufferSize = defaultBufferSize}) {
    _checkInitialized();

    final textPtr = japaneseText.toNativeUtf8();
    final buffer = malloc<ffi.Uint8>(bufferSize);
    final timePtr = malloc<ffi.Int64>();

    try {
      final length = _convert!(textPtr, buffer, bufferSize, timePtr);

      if (length < 0) {
        // Conversion failed
        return null;
      }

      // ✅ FIX: Properly decode UTF-8 bytes from C++ (not UTF-16 code units!)
      // The C++ library returns UTF-8 encoded bytes, so we must use utf8.decode()
      // instead of String.fromCharCodes() which treats bytes as UTF-16 code units
      final result = buffer.asTypedList(length);
      final phonemes = utf8.decode(result);
      final time = timePtr.value;

      return ConversionResult(
        phonemes: phonemes,
        processingTimeMicroseconds: time,
      );
    } finally {
      malloc.free(textPtr);
      malloc.free(buffer);
      malloc.free(timePtr);
    }
  }

  /// Convert Japanese text to phonemes, throwing exception on failure.
  ///
  /// Unlike [convert], this method throws a [PhonemeException] if conversion fails.
  ///
  /// Example:
  /// ```dart
  /// try {
  ///   final result = converter.convertOrThrow('日本語');
  ///   print(result.phonemes);
  /// } catch (e) {
  ///   print('Conversion failed: $e');
  /// }
  /// ```
  ConversionResult convertOrThrow(String japaneseText, {int bufferSize = defaultBufferSize}) {
    final result = convert(japaneseText, bufferSize: bufferSize);
    if (result == null) {
      throw PhonemeException('Conversion failed: $lastError');
    }
    return result;
  }

  /// Get the last error message from the native library.
  ///
  /// Returns the error message, or empty string if no error occurred.
  String get lastError {
    _checkNotDisposed();
    final errorPtr = _getError!();
    return errorPtr.toDartString();
  }

  /// Get the number of entries loaded in the phoneme dictionary.
  ///
  /// Returns -1 if not initialized.
  int get entryCount {
    _checkNotDisposed();
    return _getEntryCount!();
  }

  /// Get the version of the native library.
  String get version {
    _checkNotDisposed();
    final versionPtr = _version!();
    return versionPtr.toDartString();
  }

  /// Whether the converter has been initialized with a dictionary.
  bool get isInitialized => _isInitialized && !_isDisposed;

  /// Whether the converter has been disposed.
  bool get isDisposed => _isDisposed;

  /// Load word dictionary for word segmentation.
  ///
  /// This enables automatic word boundary detection, which adds spaces between
  /// words in the phoneme output. The dictionary file should contain one word
  /// per line in UTF-8 encoding.
  ///
  /// Throws [PhonemeException] if loading fails.
  ///
  /// Example:
  /// ```dart
  /// try {
  ///   converter.loadWordDictionary('assets/ja_words.txt');
  ///   print('Word dictionary loaded! Word count: ${converter.wordCount}');
  /// } catch (e) {
  ///   print('Failed to load dictionary: $e');
  /// }
  /// ```
  void loadWordDictionary(String wordFilePath) {
    _checkNotDisposed();

    final pathPtr = wordFilePath.toNativeUtf8();
    try {
      final result = _initWordDict!(pathPtr);
      if (result != 1) {
        throw PhonemeException('Failed to load word dictionary: $lastError');
      }
    } finally {
      malloc.free(pathPtr);
    }
  }

  /// Enable or disable word segmentation.
  ///
  /// When enabled, the converter will add spaces between words in the output.
  /// Word dictionary must be loaded via [loadWordDictionary] first.
  ///
  /// Default: `true` (enabled)
  ///
  /// Example:
  /// ```dart
  /// converter.setUseSegmentation(false);  // Disable spaces
  /// converter.setUseSegmentation(true);   // Enable spaces
  /// ```
  void setUseSegmentation(bool enabled) {
    _checkNotDisposed();
    _setUseSegmentation!(enabled);
  }

  /// Check if word segmentation is currently enabled.
  ///
  /// Returns `true` if enabled, `false` otherwise.
  bool get useSegmentation {
    _checkNotDisposed();
    return _getUseSegmentation!();
  }

  /// Get the number of words loaded in the word dictionary.
  ///
  /// Returns -1 if no word dictionary has been loaded.
  int get wordCount {
    _checkNotDisposed();
    return _getWordCount!();
  }

  /// Clean up native resources.
  ///
  /// This should be called when done using the converter.
  /// After calling dispose(), the converter cannot be used anymore.
  void dispose() {
    if (_isDisposed) return;

    _cleanup?.call();
    _isDisposed = true;
    _isInitialized = false;
  }

  /// Check that converter is initialized
  void _checkInitialized() {
    _checkNotDisposed();
    if (!_isInitialized) {
      throw PhonemeException(
        'Converter not initialized. Call init() with a dictionary file first.',
      );
    }
  }

  /// Check that converter is not disposed
  void _checkNotDisposed() {
    if (_isDisposed) {
      throw PhonemeException('Converter has been disposed and cannot be used.');
    }
  }
}

