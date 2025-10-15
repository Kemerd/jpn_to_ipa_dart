# Japanese Phoneme Converter

**Blazing fast Japanese to IPA phoneme converter** - Flutter FFI Plugin

Converts Japanese text (Hiragana, Katakana, Kanji) to International Phonetic Alphabet (IPA) representation with **microsecond-level** conversion times using native C++ via FFI.

## Features

- ⚡ **Ultra-Fast**: Microsecond-level conversion using optimized trie structure
- 🌍 **Cross-Platform**: Windows, Linux, macOS, Android, iOS - **auto-builds for all platforms!**
- 📦 **Flutter Plugin**: Just add to pubspec.yaml - native library builds automatically
- 🎯 **Type-Safe**: Full Dart type safety with null safety support
- 🔍 **Detailed Results**: Get phonemes + processing time for every conversion
- 🧵 **Thread-Safe**: Safe for concurrent operations after initialization
- 📚 **Complete Dictionary**: 200,000+ phoneme entries included
- 🚀 **No Manual Building**: Works like any other Flutter plugin
- ✂️ **Word Segmentation**: Automatic word boundary detection with 147k+ word dictionary (adds spaces between words!)

## Installation

Add to your `pubspec.yaml`:

```yaml
dependencies:
  japanese_phoneme_converter:
    git:
      url: https://github.com/Kemerd/japanese-phoneme-converter.git
      ref: main  # or a specific tag like v1.0.0
```

**That's it!** The native library will auto-build for your platform when you run `flutter run` or `flutter build`.

### Dictionary File Setup

You'll need the `ja_phonemes.json` dictionary file (~6.7MB) for phoneme conversion. Optionally, include `ja_words.txt` (~3MB, 147k+ words) for word segmentation.

**Option 1: Download to your app's assets**
```yaml
# In your app's pubspec.yaml
flutter:
  assets:
    - assets/ja_phonemes.json
    - assets/ja_words.txt      # Optional: for word segmentation
```

Download both files from this repo's `assets/` folder and place them in your app's `assets/` directory.

**Option 2: Use from package** (if you want to bundle it)
```dart
// Reference the dictionaries from the package
final packagePath = 'packages/japanese_phoneme_converter/assets/ja_phonemes.json';
converter.init(packagePath);

// Optional: Load word dictionary for segmentation
final wordPath = 'packages/japanese_phoneme_converter/assets/ja_words.txt';
converter.loadWordDictionary(wordPath);
```

### No Manual Building Required!

Unlike traditional FFI packages, this is a **proper Flutter FFI plugin**:
- ✅ Android: Builds automatically via Gradle/CMake
- ✅ iOS: Builds automatically via CocoaPods/CMake  
- ✅ Windows: Builds automatically via CMake
- ✅ macOS: Builds automatically via CMake
- ✅ Linux: Builds automatically via CMake

Just add the dependency and go!

## Quick Start

```dart
import 'package:japanese_phoneme_converter/japanese_phoneme_converter.dart';

void main() {
  // Create converter instance
  final converter = JapanesePhonemeConverter();
  
  // Initialize with dictionary
  if (!converter.init('assets/ja_phonemes.json')) {
    print('Failed to initialize: ${converter.lastError}');
    return;
  }
  
  // Optional: Load word dictionary for word segmentation
  try {
    converter.loadWordDictionary('assets/ja_words.txt');
    print('Word segmentation enabled! Loaded ${converter.wordCount} words');
  } catch (e) {
    print('Word segmentation disabled: $e');
  }
  
  // Convert Japanese text (with automatic word spacing if dictionary loaded!)
  final result = converter.convert('私はリンゴが好きです');
  if (result != null) {
    print('Phonemes: ${result.phonemes}');  // Output: "ɰᵝatai ha ɾiɴgo ga sɯki desɯ" (with spaces!)
    print('Time: ${result.processingTimeMicroseconds}μs');
  }
  
  // Clean up
  converter.dispose();
}
```

---

## Usage Examples

### Basic Conversion

```dart
final converter = JapanesePhonemeConverter();
converter.init('assets/ja_phonemes.json');

final result = converter.convert('日本語');
print(result?.phonemes); // IPA phonemes
```

### Word Segmentation (NEW! ✨)

Automatically add spaces between words for better readability and text-to-speech:

```dart
final converter = JapanesePhonemeConverter();
converter.init('assets/ja_phonemes.json');

// Load word dictionary (147k+ words)
converter.loadWordDictionary('assets/ja_words.txt');

// Convert with automatic word spacing
final result = converter.convert('今日はいい天気ですね');
print(result?.phonemes); 
// Output: "kʲoː wa i teɴki desɯ ne" (with spaces between words!)

// Toggle segmentation on/off
converter.setUseSegmentation(false);  // Disable spaces
final noSpaces = converter.convert('今日はいい天気ですね');
print(noSpaces?.phonemes);
// Output: "kʲoːwaiteɴkidesɯne" (no spaces)

converter.setUseSegmentation(true);   // Re-enable

// Check status
print('Segmentation enabled: ${converter.useSegmentation}');
print('Words loaded: ${converter.wordCount}');
```

**How it works:**
- Matches known words from 147k+ word dictionary
- Treats unmatched text between words as grammar particles (は、が、を、です, etc.)
- Both words AND grammar get spaces → perfect for TTS and tokenization!

### With Error Handling

```dart
try {
  final result = converter.convertOrThrow('東京');
  print('Phonemes: ${result.phonemes}');
  print('Time: ${result.processingTimeMicroseconds}μs');
} on PhonemeException catch (e) {
  print('Conversion failed: $e');
}
```

### Performance Metrics

```dart
final result = converter.convert('ありがとうございます');
if (result != null) {
  print('Microseconds: ${result.processingTimeMicroseconds}');
  print('Milliseconds: ${result.processingTimeMilliseconds}');
}
```

### Batch Processing

```dart
final texts = ['こんにちは', '日本語', '東京'];
for (final text in texts) {
  final result = converter.convert(text);
  if (result != null) {
    print('$text → ${result.phonemes}');
  }
}
```

### Getting Dictionary Info

```dart
print('Library version: ${converter.version}');
print('Entries loaded: ${converter.entryCount}');
print('Initialized: ${converter.isInitialized}');
```

### Custom Library Path

```dart
final converter = JapanesePhonemeConverter(
  libraryPath: 'path/to/jpn_to_phoneme_ffi.dll'
);
```

---

## API Reference

### JapanesePhonemeConverter

Main class for phoneme conversion operations.

#### Constructor

```dart
JapanesePhonemeConverter({String? libraryPath})
```

Creates a new converter instance. If `libraryPath` is not provided, the library is loaded automatically based on the current platform.

#### Methods

**`bool init(String jsonFilePath)`**

Initialize the converter with a phoneme dictionary JSON file. Must be called before any conversion operations.

- Returns: `true` on success, `false` on failure
- Example: `converter.init('assets/ja_phonemes.json')`

**`ConversionResult? convert(String text, {int bufferSize = 4096})`**

Convert Japanese text to IPA phonemes.

- Returns: `ConversionResult` on success, `null` on failure
- Parameters:
  - `text`: Input Japanese text
  - `bufferSize`: Output buffer size (default 4KB)

**`ConversionResult convertOrThrow(String text, {int bufferSize = 4096})`**

Convert Japanese text to phonemes, throwing exception on failure.

- Returns: `ConversionResult`
- Throws: `PhonemeException` on failure

**`void dispose()`**

Clean up native resources. Must be called when done using the converter.

**`void loadWordDictionary(String wordFilePath)`** ✨ NEW

Load word dictionary for word segmentation. Enables automatic word boundary detection.

- Parameters:
  - `wordFilePath`: Path to ja_words.txt file
- Throws: `PhonemeException` if file not found or loading fails
- Example: `converter.loadWordDictionary('assets/ja_words.txt')`

**`void setUseSegmentation(bool enabled)`** ✨ NEW

Enable or disable word segmentation at runtime.

- Parameters:
  - `enabled`: true to enable spaces between words, false to disable
- Note: Word dictionary must be loaded first
- Example: `converter.setUseSegmentation(false)`

#### Properties

- **`String version`** - Native library version
- **`String lastError`** - Last error message from native library
- **`int entryCount`** - Number of dictionary entries loaded (-1 if not initialized)
- **`bool isInitialized`** - Whether converter is initialized and ready
- **`bool isDisposed`** - Whether converter has been disposed
- **`bool useSegmentation`** ✨ NEW - Whether word segmentation is currently enabled
- **`int wordCount`** ✨ NEW - Number of words loaded in dictionary (-1 if not loaded)

### ConversionResult

Data class containing conversion results and performance metrics.

#### Properties

- **`String phonemes`** - The converted IPA phoneme string
- **`int processingTimeMicroseconds`** - Processing time in microseconds
- **`double processingTimeMilliseconds`** - Processing time in milliseconds

#### Methods

- `toString()` - String representation of the result
- `operator ==` - Equality comparison
- `hashCode` - Hash code for the result

### PhonemeException

Exception thrown when phoneme conversion operations fail.

#### Properties

- **`String message`** - Error message describing what went wrong

---

## Package Structure

```
japanese_phoneme_converter/
├── lib/
│   ├── japanese_phoneme_converter.dart      # Main export file
│   └── src/
│       ├── japanese_phoneme_converter.dart  # Core converter class
│       ├── conversion_result.dart           # Result data class
│       └── phoneme_exception.dart           # Exception types
├── native/
│   ├── jpn_to_phoneme_ffi.cpp              # C++ source (shared across platforms)
│   └── CMakeLists.txt                      # Standalone build config (optional)
├── android/
│   └── CMakeLists.txt                      # Android build configuration
├── ios/
│   └── japanese_phoneme_converter.podspec  # iOS build configuration  
├── windows/
│   └── CMakeLists.txt                      # Windows build configuration
├── linux/
│   └── CMakeLists.txt                      # Linux build configuration
├── macos/
│   └── CMakeLists.txt                      # macOS build configuration
├── assets/
│   ├── ja_phonemes.json                    # Phoneme dictionary (220k+ entries, ~7.5MB)
│   └── ja_words.txt                        # Word dictionary (147k+ words, ~3MB) ✨ NEW
├── test/
│   └── japanese_phoneme_converter_test.dart # Unit tests
├── example/
│   └── example.dart                        # Usage examples
├── pubspec.yaml                            # Package metadata
├── CHANGELOG.md                            # Version history
├── LICENSE                                 # MIT License
└── README.md                               # This file
```

---

## Performance

Typical conversion times on modern hardware:

| Text Length | Conversion Time |
|-------------|-----------------|
| 10 chars    | ~5-10 μs        |
| 100 chars   | ~50-100 μs      |
| 1000 chars  | ~500-1000 μs    |

*Benchmarks on Intel i7-8700K*

### Optimization Details

The library uses aggressive optimizations:
- **MSVC**: `/O2 /Ob2 /Oi /Ot /GL` (whole program optimization)
- **GCC/Clang**: `-O3 -march=native -ffast-math` (native CPU optimizations)
- **Algorithm**: Trie structure with pre-decoded UTF-8 for 10x speed boost
- **Memory**: Dictionary loaded once, ~10-20MB in memory

---

## Troubleshooting

### Library Not Found Error

**Problem**: `Failed to load native library`

**Solution**: This shouldn't happen with the FFI plugin setup. If it does:

1. Make sure you're running in a Flutter app context (`flutter run`), not pure Dart
2. Try `flutter clean` and rebuild
3. For unit tests, the native library needs to be pre-built (tests run in Dart VM, not Flutter runtime)

### Initialization Failed

**Problem**: `init()` returns `false`

**Solution**: Check the dictionary file path and error message:

```dart
if (!converter.init('assets/ja_phonemes.json')) {
  print('Error: ${converter.lastError}');
  // Common issues:
  // - File doesn't exist at the specified path
  // - File is not valid JSON
  // - Insufficient permissions to read file
}
```

### UTF-8 Encoding Issues

**Problem**: Incorrect phoneme output or conversion errors

**Solution**: Ensure all files are UTF-8 encoded:
- Dart source files: Save as UTF-8
- Dictionary JSON: Must be UTF-8 encoded
- Input text: Should be UTF-8 strings (Dart default)

### Build Errors

**Problem**: Native compilation fails during `flutter build`

**Solution**:
- **Windows**: Ensure Visual Studio 2017+ with C++ tools is installed
- **Linux**: Install build tools: `sudo apt-get install build-essential cmake`
- **macOS**: Install Xcode command line tools: `xcode-select --install`
- **Android**: Ensure NDK is installed via Android Studio
- **iOS**: Ensure Xcode is installed and up to date

---

## Testing

Run the test suite:

```bash
dart test
```

### Test Coverage

- ✅ Library loading and initialization
- ✅ Basic text conversion
- ✅ Error handling and exceptions
- ✅ Thread safety and concurrent operations
- ✅ Resource cleanup and disposal
- ✅ Edge cases (empty strings, large text)
- ✅ Performance metrics accuracy
- ✅ Word dictionary loading and segmentation ✨ NEW
- ✅ Segmentation enable/disable toggle ✨ NEW
- ✅ Space-separated output verification ✨ NEW

---

## Example Application

See `example/example.dart` for a complete working example:

```bash
# From the dart_ffi directory
dart run example/example.dart
```

The example demonstrates:
- Loading and initializing the converter
- Loading word dictionary for segmentation ✨ NEW
- Converting with and without word segmentation ✨ NEW
- Converting multiple Japanese texts
- Measuring performance
- Error handling
- Proper resource cleanup

---

## Word Segmentation Feature ✨

**NEW**: Automatic word boundary detection with space-separated output!

### What is Word Segmentation?

Word segmentation automatically splits Japanese text into words and adds spaces between them in the phoneme output. This is incredibly useful for:

- **Text-to-Speech (TTS)**: Natural pauses at word boundaries
- **Tokenization**: Space-delimited output for downstream processing
- **Linguistic Analysis**: Automatic morpheme detection
- **Training Data**: Pre-segmented text for machine learning

### How It Works

The system uses a **two-pass algorithm**:

1. **Pass 1 - Word Segmentation**: Split Japanese text into tokens using a 147k+ word dictionary
2. **Pass 2 - Phoneme Conversion**: Convert each token to phonemes and add spaces between them

### Smart Grammar Detection

The segmenter automatically identifies grammatical elements by treating any text between known words as grammar:

**Example**: `私はリンゴが好きです`

**Dictionary Matches** (words):
- `私` (watashi) - WORD
- `リンゴ` (ringo) - WORD  
- `好き` (suki) - WORD

**Unmatched Between Words** (automatically treated as grammar):
- `は` (ha) - particle
- `が` (ga) - particle
- `です` (desu) - copula

**Result**: `私` `は` `リンゴ` `が` `好き` `です`  
**Output**: `ɰᵝatai ha ɾiɴgo ga sɯki desɯ` (with spaces!)

### Performance Impact

- **Additional Loading**: +50-100ms one-time (loads 147k words)
- **Conversion Speed**: ~same as before (still <1ms per sentence)
- **Memory**: +20MB (word dictionary trie)

### Usage

```dart
// Load word dictionary (throws on failure)
converter.loadWordDictionary('assets/ja_words.txt');

// Convert with segmentation (default: enabled)
var result = converter.convert('今日はいい天気ですね');
print(result?.phonemes); // "kʲoː wa i teɴki desɯ ne" (with spaces!)

// Toggle on/off at runtime
converter.setUseSegmentation(false);  // Disable
converter.setUseSegmentation(true);   // Enable

// Check status
print('Enabled: ${converter.useSegmentation}');
print('Words loaded: ${converter.wordCount}');
```

---

## Advanced Usage

### Thread Safety

The converter is thread-safe for conversions after initialization:

```dart
final converter = JapanesePhonemeConverter();
converter.init('assets/ja_phonemes.json');

// Safe to call from multiple isolates
await Future.wait([
  compute(convertText, 'こんにちは'),
  compute(convertText, '日本語'),
  compute(convertText, '東京'),
]);
```

### Custom Buffer Size

For very long text, increase the buffer size:

```dart
final result = converter.convert(
  veryLongText,
  bufferSize: 1024 * 1024, // 1MB buffer
);
```

### Integration with Flutter

```dart
class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  late JapanesePhonemeConverter _converter;

  @override
  void initState() {
    super.initState();
    _converter = JapanesePhonemeConverter();
    _converter.init('assets/ja_phonemes.json');
  }

  @override
  void dispose() {
    _converter.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(/* ... */);
  }
}
```

---

## Size and Dependencies

- **Native library**: ~50-100 KB (platform-dependent)
- **Phoneme dictionary**: ~7.5 MB (ja_phonemes.json, 220k+ entries)
- **Word dictionary**: ~3 MB (ja_words.txt, 147k+ words) - Optional ✨
- **Dart code**: ~20 KB
- **Total package**: ~7-11 MB (depending on whether you include word dictionary)

### Runtime Dependencies

- `ffi: ^2.1.0` - Foreign Function Interface support
- `flutter` - Flutter SDK

### No External Dependencies

The native library is self-contained with no external C++ dependencies or system libraries required.

---

## License

MIT License - See [LICENSE](LICENSE) file for details

Copyright (c) 2025 Kemerd

---

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

### Development Setup

```bash
# Get dependencies
dart pub get

# Run tests
dart test

# Format code
dart format .

# Analyze code
dart analyze
```

---

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history.

---

## Support

- 📧 Issues: https://github.com/Kemerd/japanese-phoneme-converter/issues
- 📚 Documentation: This README
- 💬 Discussions: GitHub Discussions

---

## Credits

Created with ❤️ for fast, efficient Japanese text processing.

Based on the high-performance C++ implementation with optimized trie structure for maximum speed.
