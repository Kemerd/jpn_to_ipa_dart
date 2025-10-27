import 'dart:convert'; // For utf8.decode
import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart'; // For rootBundle

import 'conversion_result.dart';
import 'phoneme_exception.dart';
import 'japanese_phoneme_converter_bindings_generated.dart';

// Export public API
export 'conversion_result.dart';
export 'phoneme_exception.dart';

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
  JapanesePhonemeConverterBindings? _bindings;

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

  /// Bind native functions using generated bindings
  void _bindFunctions() {
    _bindings = JapanesePhonemeConverterBindings(_lib!);
  }

  /// Initialize the converter with the binary trie.
  ///
  /// By default, loads `japanese.trie` bundled with this plugin.
  /// No setup required in your app - the asset is included automatically!
  ///
  /// You can override with a custom asset path if needed.
  ///
  /// Returns `true` on success, `false` on failure.
  ///
  /// Example:
  /// ```dart
  /// // Use bundled asset (default)
  /// if (!await converter.init()) {
  ///   print('Failed: ${converter.lastError}');
  /// }
  ///
  /// // Or use custom asset
  /// if (!await converter.init(assetPath: 'assets/my_custom.trie')) {
  ///   print('Failed: ${converter.lastError}');
  /// }
  /// ```
  Future<bool> init({String? assetPath}) async {
    _checkNotDisposed();

    try {
      // Default to plugin's bundled asset using package: scheme
      final path = assetPath ?? 'packages/japanese_phoneme_converter/assets/japanese.trie';
      
      // Load trie from Flutter assets
      final trieData = await rootBundle.load(path);
      
      // Convert to Uint8List with proper offset/length handling
      final trieBytes = trieData.buffer.asUint8List(
        trieData.offsetInBytes,
        trieData.lengthInBytes,
      );
      
      // Pass to native code
      return initFromMemory(trieBytes);
    } catch (e) {
      // Store error for lastError getter
      _lastInitError = 'Failed to load asset $assetPath: $e';
      return false;
    }
  }
  
  String _lastInitError = '';
  
  /// Initialize the converter from file path (legacy method).
  /// Prefer using [init] which loads from assets automatically.
  @Deprecated('Use init() instead which loads from assets automatically')
  bool initFromPath(String jsonFilePath) {
    _checkNotDisposed();

    final pathPtr = jsonFilePath.toNativeUtf8();
    try {
      final result = _bindings!.jpn_phoneme_init(pathPtr.cast());
      _isInitialized = result == 1;
      return _isInitialized;
    } finally {
      malloc.free(pathPtr);
    }
  }

  /// Initialize the converter from .trie data loaded in memory.
  /// 🔥 BLAZING FAST: Use this to load .trie directly from Flutter assets!
  ///
  /// This is the preferred method for Flutter apps as it avoids file system access.
  /// Load your .trie asset using rootBundle.load() and pass the bytes here.
  ///
  /// Returns `true` on success, `false` on failure.
  ///
  /// Example:
  /// ```dart
  /// import 'package:flutter/services.dart';
  /// 
  /// final data = await rootBundle.load('assets/japanese.trie');
  /// if (!converter.initFromMemory(data.buffer.asUint8List())) {
  ///   print('Failed: ${converter.lastError}');
  /// }
  /// ```
  bool initFromMemory(List<int> trieData) {
    _checkNotDisposed();

    // Allocate native memory for the data
    final dataPtr = malloc<ffi.Uint8>(trieData.length);
    try {
      // Copy Dart list to native memory
      final nativeList = dataPtr.asTypedList(trieData.length);
      nativeList.setAll(0, trieData);
      
      // Call native function
      final result = _bindings!.jpn_phoneme_init_from_memory(dataPtr, trieData.length);
      _isInitialized = result == 1;
      return _isInitialized;
    } finally {
      // Free the allocated memory
      malloc.free(dataPtr);
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
      final length = _bindings!.jpn_phoneme_convert(textPtr.cast(), buffer, bufferSize, timePtr);

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
    final errorPtr = _bindings!.jpn_phoneme_get_error();
    return errorPtr.cast<Utf8>().toDartString();
  }

  /// Get the number of entries loaded in the phoneme dictionary.
  ///
  /// Returns -1 if not initialized.
  int get entryCount {
    _checkNotDisposed();
    return _bindings!.jpn_phoneme_get_entry_count();
  }

  /// Get the version of the native library.
  String get version {
    _checkNotDisposed();
    final versionPtr = _bindings!.jpn_phoneme_version();
    return versionPtr.cast<Utf8>().toDartString();
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
      final result = _bindings!.jpn_phoneme_init_word_dict(pathPtr.cast());
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
    _bindings!.jpn_phoneme_set_use_segmentation(enabled);
  }

  /// Check if word segmentation is currently enabled.
  ///
  /// Returns `true` if enabled, `false` otherwise.
  bool get useSegmentation {
    _checkNotDisposed();
    return _bindings!.jpn_phoneme_get_use_segmentation();
  }

  /// Get the number of words loaded in the word dictionary.
  ///
  /// Returns -1 if no word dictionary has been loaded.
  int get wordCount {
    _checkNotDisposed();
    return _bindings!.jpn_phoneme_get_word_count();
  }

  /// Clean up native resources.
  ///
  /// This should be called when done using the converter.
  /// After calling dispose(), the converter cannot be used anymore.
  void dispose() {
    if (_isDisposed) return;

    _bindings?.jpn_phoneme_cleanup();
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

