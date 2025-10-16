# Japanese Phoneme Converter

**Blazing fast Japanese to IPA phoneme converter** - Flutter FFI Plugin

Converts Japanese text (Hiragana, Katakana, Kanji) to International Phonetic Alphabet (IPA) representation with **microsecond-level** conversion times using native C++ via FFI.

## Features

- ⚡ **Ultra-Fast**: Microsecond-level conversion using optimized trie structure
- 🚀 **Binary Trie Format**: 100x faster loading with `.trie` format (auto-fallback to JSON)
- 🌍 **Cross-Platform**: Windows, Linux, macOS, Android, iOS - **auto-builds for all platforms!**
- 📦 **Flutter Plugin**: Just add to pubspec.yaml - native library builds automatically
- 🎯 **Type-Safe**: Full Dart type safety with null safety support
- 🔍 **Detailed Results**: Get phonemes + processing time for every conversion
- 🧵 **Thread-Safe**: Safe for concurrent operations after initialization
- 📚 **Complete Dictionary**: 474k+ entries (phonemes + words in unified trie)
- 🚀 **No Manual Building**: Works like any other Flutter plugin
- ✂️ **Word Segmentation**: Automatic word boundary detection with phoneme fallback
- 🎌 **Furigana Hints**: Smart pronunciation hints with compound word detection & okurigana handling

## Installation

Add to your `pubspec.yaml`:

```yaml
dependencies:
  japanese_phoneme_converter:
    git:
      url: https://github.com/Kemerd/jpn_to_ipa_dart.git
      ref: main  # or a specific tag like v1.0.0
```

**That's it!** The native library will auto-build for your platform when you run `flutter run` or `flutter build`.

### Dictionary File Setup

**Use Binary Format (.trie) - Required for Production**
```yaml
# In your app's pubspec.yaml
flutter:
  assets:
    - assets/japanese.trie    # Binary format - 474k+ entries, 100x faster!
```

That's it! The `.trie` file includes everything:
- ✅ 474k+ entries (phonemes + words unified)
- ✅ 100x faster loading (200-300ms vs 2-5s)
- ✅ Smaller file size (~5.5MB vs ~10.5MB for JSON+words)
- ✅ Word segmentation built-in

**Don't ship JSON files in production!** The binary format is the only format you should use.

### No Manual Building Required!

Unlike traditional FFI packages, this is a **proper Flutter FFI plugin**:
- ✅ Android: Builds automatically via Gradle/CMake
- ✅ iOS: Builds automatically via CocoaPods/CMake  
- ✅ Windows: Builds automatically via CMake
- ✅ macOS: Builds automatically via CMake
- ✅ Linux: Builds automatically via CMake

Just add the dependency and go!

## Quick Start

### ⚡ IMPORTANT: For Flutter Apps, Use `initFromMemory()`!

**Flutter apps CANNOT use file paths** because native C++ code cannot access Flutter's bundled assets directly. You **MUST** load the `.trie` file into memory first:

```dart
import 'package:flutter/services.dart';
import 'package:japanese_phoneme_converter/japanese_phoneme_converter.dart';

Future<void> example() async {
  // Create converter instance
  final converter = JapanesePhonemeConverter();
  
  // 🔥 REQUIRED: Load .trie from assets into memory
  final trieData = await rootBundle.load('assets/japanese.trie');
  
  if (!converter.initFromMemory(trieData.buffer.asUint8List())) {
    print('Failed to initialize: ${converter.lastError}');
    return;
  }
  
  print('✅ Loaded ${converter.entryCount} entries');
  print('✅ Word segmentation: auto-enabled');
  
  // Convert Japanese text with automatic word spacing!
  final result = converter.convert('私はリンゴが好きです');
  if (result != null) {
    print('Phonemes: ${result.phonemes}');
    print('Time: ${result.processingTimeMicroseconds}μs');
  }
  
  // Clean up
  converter.dispose();
}
```

### For Non-Flutter Dart Apps (CLI Tools)

If you're NOT using Flutter, you can use file paths:

```dart
import 'package:japanese_phoneme_converter/japanese_phoneme_converter.dart';

void main() {
  final converter = JapanesePhonemeConverter();
  
  // File path works for non-Flutter apps
  if (!converter.init('path/to/japanese.trie')) {
    print('Failed to initialize: ${converter.lastError}');
    return;
  }
  
  final result = converter.convert('日本語');
  print(result?.phonemes);
  
  converter.dispose();
}
```

---

## Usage Examples

### Basic Conversion (Flutter)

```dart
import 'package:flutter/services.dart';

Future<void> convertText() async {
  final converter = JapanesePhonemeConverter();
  
  // 🔥 Load from memory (REQUIRED for Flutter!)
  final trieData = await rootBundle.load('assets/japanese.trie');
  converter.initFromMemory(trieData.buffer.asUint8List());

  final result = converter.convert('日本語');
  print(result?.phonemes); // IPA phonemes with automatic word spacing
  
  converter.dispose();
}
```

### Word Segmentation (Automatic! ✨)

The `.trie` format includes word segmentation automatically:

```dart
import 'package:flutter/services.dart';

Future<void> segmentationExample() async {
  final converter = JapanesePhonemeConverter();
  
  // 🔥 Load from memory (REQUIRED for Flutter!)
  final trieData = await rootBundle.load('assets/japanese.trie');
  converter.initFromMemory(trieData.buffer.asUint8List());

  // Convert with automatic word spacing (enabled by default)
  final result = converter.convert('今日はいい天気ですね');
  print(result?.phonemes); 
  // Output: "kʲoː wa i teɴki desɯ ne" (with spaces between words!)

  // Toggle segmentation on/off if needed
  converter.setUseSegmentation(false);  // Disable spaces
  final noSpaces = converter.convert('今日はいい天気ですね');
  print(noSpaces?.phonemes);
  // Output: "kʲoːwaiteɴkidesɯne" (no spaces)

  converter.setUseSegmentation(true);   // Re-enable

  // Check status
  print('Segmentation enabled: ${converter.useSegmentation}');
  print('Entries loaded: ${converter.entryCount}'); // 474k+ entries!
  
  converter.dispose();
}
```

**How it works:**
- Unified trie with 474k+ entries (phonemes + words together)
- Smart phoneme fallback: If word dict has no match, tries phoneme dict
- Automatic grammar detection: Treats unmatched text as particles (は、が、を、です, etc.)
- Space-separated output → perfect for TTS and tokenization!

**No separate files needed!** Everything is in `japanese.trie`.

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

**`bool init(String trieFilePath)`**

Initialize the converter with the binary trie file. Must be called before any conversion operations.

- Returns: `true` on success, `false` on failure
- Example: `converter.init('assets/japanese.trie')`
- **Note**: Always use `.trie` format for production apps!

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

**`void loadWordDictionary(String wordFilePath)`** (Not needed!)

**Deprecated**: This method is only for legacy JSON-based loading. When using `japanese.trie`, word segmentation is already included.

- **You don't need this!** The `.trie` format includes everything.

**`void setUseSegmentation(bool enabled)`** ✨ NEW

Enable or disable word segmentation at runtime.

- Parameters:
  - `enabled`: true to enable spaces between words, false to disable
- Note: Word dictionary must be loaded first
- Example: `converter.setUseSegmentation(false)`

#### Properties

- **`String version`** - Native library version
- **`String lastError`** - Last error message from native library
- **`int entryCount`** - Number of dictionary entries loaded (474k+ if using binary format, 220k+ if JSON-only)
- **`bool isInitialized`** - Whether converter is initialized and ready
- **`bool isDisposed`** - Whether converter has been disposed
- **`bool useSegmentation`** - Whether word segmentation is currently enabled
- **`int wordCount`** - Number of words in word-only dictionary (only if ja_words.txt loaded separately)

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
│   └── japanese.trie                       # Binary format (474k+ entries, ~5.5MB) - USE THIS! ✨
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

### Loading Time

**Binary Format (.trie)**: 247-252ms for 474k+ entries

- 100x faster than JSON parsing
- Includes phonemes + words unified
- No additional loading needed!

### Conversion Time

Typical conversion times on modern hardware:

| Text Length | Conversion Time |
|-------------|-----------------|
| 10 chars    | ~5-10 μs        |
| 100 chars   | ~50-100 μs      |
| 1000 chars  | ~500-1000 μs    |

*Benchmarks on Intel i7-8700K*

**Always use `japanese.trie` for production!** Don't ship JSON files.

### Optimization Details

The library uses aggressive optimizations:
- **MSVC**: `/O2 /Ob2 /Oi /Ot /GL` (whole program optimization)
- **GCC/Clang**: `-O3 -march=native -ffast-math` (native CPU optimizations)
- **Binary Format**: Custom JPHO format with varint encoding for ultra-fast loading
- **Algorithm**: Unified trie structure with pre-decoded UTF-8 for 10x speed boost
- **Memory**: Dictionary loaded once, ~30-50MB in memory (474k+ entries)

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

**Solution**: Check the trie file path and error message:

```dart
if (!converter.init('assets/japanese.trie')) {
  print('Error: ${converter.lastError}');
  // Common issues:
  // - File doesn't exist at the specified path
  // - File is not the correct binary format
  // - Insufficient permissions to read file
}
```

### UTF-8 Encoding Issues

**Problem**: Incorrect phoneme output or conversion errors

**Solution**: Ensure all files are UTF-8 encoded:
- Dart source files: Save as UTF-8
- Binary trie: Pre-encoded as UTF-8 (no issues!)
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

## Furigana Hint Support 🎯

**💡 Tip for Names**: Furigana hints are especially useful for proper names (people, places) that aren't in the dictionary or have non-standard readings. They ensure correct pronunciation and proper particle separation!


### What is Furigana Hint Support?

Use furigana brackets `「」` to provide pronunciation hints for names or words not in the dictionary, with **smart compound word detection** and **okurigana handling** that automatically prioritizes dictionary entries when appropriate!

### The Problem

Names and uncommon words often get incorrectly segmented because they're not in the dictionary:

```dart
// Without hints: けんたは gets segmented as one chunk
converter.convert('けんたはバカ');
// Output: "keɴtaha baka" ❌ (は particle attached to name)
```

### The Solution

Use furigana hints with kanji or unknown text:

```dart
// With furigana hint: proper particle separation!
converter.convert('健太「けんた」はバカ');
// Output: "keɴta ha baka" ✅ (proper separation!)
```

### How It Works

The system uses **TextSegment-based processing** with advanced features:

1. **Pre-decode UTF-8**: Convert text to code points for blazing speed
2. **Smart Okurigana Detection**:
   - その男「おとこ」 → Captures only 男 (stops at kana prefix "その")
   - 昼ご飯「ひるごはん」 → Captures all 昼ご飯 (keeps sandwiched kana "ご")
   - Algorithm: Two-pass kanji boundary detection with sandwiching logic
3. **Compound Word Detection**: Uses trie longest-match to check if kanji + following text forms a dictionary word
4. **Smart Decision**:
   - If compound found → use dictionary word (e.g., 見「み」て → 見て)
   - If no compound → use furigana reading (e.g., 健太「けんた」→ けんた)
5. **TextSegment Processing**: Furigana segments treated as atomic units during word segmentation
6. **Phoneme Conversion**: Each segment converted to phonemes with spaces between words

**Example 1** - Compound word prioritization:
```dart
converter.convert('見「み」て');
// → Trie detects 見て is in dictionary
// → Uses 見て from dictionary (ignores hint)
// Output: "mite" ✅
```

**Example 2** - Name with hint:
```dart
converter.convert('健太「けんた」さん');
// → Checks if 健太さん is in dictionary via trie → NO
// → Uses furigana reading → TextSegment(reading: "けんた")
// → Segments as: [けんた] [さん]
// Output: "keɴta saɴ" ✅
```

**Example 3** - Okurigana handling:
```dart
converter.convert('その男「おとこ」が好き');
// → Smart boundary detection stops at "その"
// → Captures only "男" for the hint
// → Output: "sono otoko ga sɯki" ✅
```

### Usage Examples

#### Basic Name
```dart
converter.convert('健太「けんた」はバカです');
// Output: "keɴta ha baka desɯ"
```

#### Multiple Names
```dart
converter.convert('健太「けんた」と雪「ゆき」が好き');
// Output: "keɴta to jɯki ga sɯki"
```

#### Complex Sentence
```dart
converter.convert('私「わたし」は健太「けんた」が好きです');
// Output: "ɰᵝataɕi ha keɴta ga sɯki desɯ"
```

#### Katakana Names
```dart
converter.convert('ジョン「じょん」はアメリカ人です');
// Output: "ʥijoɴ ha ameɾikaʥiɴ desɯ"
```

### Advanced Features

**Smart Okurigana Detection**:
- **Prefix detection**: その男「おとこ」 → Stops at kana prefix, captures only 男
- **Sandwiched kana**: 昼ご飯「ひるごはん」 → Keeps "ご" between kanji
- **Two-pass algorithm**: First finds last kanji, then scans backwards for word boundary
- **Punctuation boundaries**: Properly handles Japanese and ASCII punctuation

**Compound Word Detection**:
- **Trie-based longest-match**: Uses word dictionary trie for efficient compound detection
- **Example**: 見「み」て → Walks trie through 見 + て, finds 見て is valid word
- **Priority**: Dictionary words always prioritized over forced readings

**TextSegment Processing**:
- **Structured representation**: Each segment is either normal text or furigana hint
- **Atomic units**: Furigana segments treated as single words during segmentation
- **Type safety**: Explicit SegmentType enum (NORMAL_TEXT, FURIGANA_HINT)

### Why This Approach Works

1. **No markers**: Clean TextSegment-based architecture, no string manipulation overhead
2. **Dictionary-first**: Trie-based compound detection ensures dictionary words prioritized
3. **Smart boundaries**: Advanced okurigana detection with two-pass algorithm
4. **Minimal overhead**: Pre-decoded UTF-8 + trie lookups = <5μs per hint
5. **Backwards compatible**: Text without hints works normally
6. **Intrinsic grammar recognition**: Particles (は、が、を) detected automatically

### Performance Impact

- Furigana processing: ~2-5 μs per sentence (pre-decoded UTF-8 + trie)
- Compound detection: <1 μs per hint (trie longest-match)
- Overall conversion: <5% overhead
- Memory: ~100 KB additional for TextSegment structures

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
    _converter.init('assets/japanese.trie');  // Use binary format!
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
- **Binary trie**: ~14.8 MB (japanese.trie, 474k+ entries - everything included!)
- **Dart code**: ~20 KB
- **Total package**: ~5.7 MB

**That's it!** No JSON, no separate word files, just the single `.trie` file.

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
