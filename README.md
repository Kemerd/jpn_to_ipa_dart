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

## Installation

Add to your `pubspec.yaml`:

```yaml
dependencies:
  japanese_phoneme_converter:
    path: ../path/to/dart_ffi  # Or git URL when published
```

**That's it!** The native library will auto-build for your platform when you run `flutter run` or `flutter build`.

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
  
  // Convert Japanese text
  final result = converter.convert('こんにちは');
  if (result != null) {
    print('Phonemes: ${result.phonemes}');
    print('Time: ${result.processingTimeMicroseconds}μs');
  }
  
  // Clean up
  converter.dispose();
}
```

---

## Building the Native Library

### Prerequisites

- **CMake** 3.15 or later: https://cmake.org/download/
- **C++17 Compiler**:
  - Windows: Visual Studio 2017+ or MinGW
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode Command Line Tools

### Quick Build

**Windows:**
```bash
cd native
build.bat
```

**Linux / macOS:**
```bash
cd native
chmod +x build.sh
./build.sh
```

### Manual Build Steps

```bash
cd native

# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build
# Windows:
cmake --build build --config Release

# Linux/macOS:
cmake --build build

# Copy output to package root
# Windows: copy build\Release\jpn_to_phoneme_ffi.dll ..\
# Linux:   cp build/jpn_to_phoneme_ffi.so ../
# macOS:   cp build/jpn_to_phoneme_ffi.dylib ../
```

### Output Files

After building, you should have:
- Windows: `dart_ffi/jpn_to_phoneme_ffi.dll`
- Linux: `dart_ffi/jpn_to_phoneme_ffi.so`
- macOS: `dart_ffi/jpn_to_phoneme_ffi.dylib`

### Android Build

For Android/Flutter apps, the native library builds automatically via Gradle.

Add to `android/app/build.gradle`:

```gradle
android {
    externalNativeBuild {
        cmake {
            path "../../native/CMakeLists.txt"
            version "3.18.1"
        }
    }
}
```

### iOS Build

Add the native source `native/jpn_to_phoneme_ffi.cpp` to your Xcode project and link against the C++ standard library.

---

## Usage Examples

### Basic Conversion

```dart
final converter = JapanesePhonemeConverter();
converter.init('assets/ja_phonemes.json');

final result = converter.convert('日本語');
print(result?.phonemes); // IPA phonemes
```

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

#### Properties

- **`String version`** - Native library version
- **`String lastError`** - Last error message from native library
- **`int entryCount`** - Number of dictionary entries loaded (-1 if not initialized)
- **`bool isInitialized`** - Whether converter is initialized and ready
- **`bool isDisposed`** - Whether converter has been disposed

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
dart_ffi/
├── lib/
│   ├── japanese_phoneme_converter.dart      # Main export file
│   └── src/
│       ├── japanese_phoneme_converter.dart  # Core converter class
│       ├── conversion_result.dart           # Result data class
│       └── phoneme_exception.dart           # Exception types
├── native/
│   ├── jpn_to_phoneme_ffi.cpp              # C++ source (FFI compatible)
│   ├── CMakeLists.txt                      # CMake build config
│   ├── build.bat                           # Windows build script
│   └── build.sh                            # Unix build script
├── assets/
│   └── ja_phonemes.json                    # Phoneme dictionary (200k+ entries)
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

**Solution**: Ensure the native library is in the correct location:

```dart
// Option 1: Specify full path
final converter = JapanesePhonemeConverter(
  libraryPath: '/full/path/to/jpn_to_phoneme_ffi.dll'
);

// Option 2: Set environment variable
// Windows: set PATH=%PATH%;C:\path\to\library
// Linux:   export LD_LIBRARY_PATH=/path/to/library:$LD_LIBRARY_PATH
// macOS:   export DYLD_LIBRARY_PATH=/path/to/library:$DYLD_LIBRARY_PATH
```

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

### CMake Configuration Errors

**Problem**: CMake fails to configure

**Solution**:
```bash
# Update CMake
# Install C++17 compiler
# On Windows: Use Visual Studio Developer Command Prompt
# On Linux: sudo apt-get install build-essential cmake
# On macOS: xcode-select --install
```

### Build Fails on Windows

**Problem**: MSBuild errors or compiler not found

**Solution**: Use Visual Studio Developer Command Prompt or specify generator:
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

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

---

## Example Application

See `example/example.dart` for a complete working example:

```bash
# From the dart_ffi directory
dart run example/example.dart
```

The example demonstrates:
- Loading and initializing the converter
- Converting multiple Japanese texts
- Measuring performance
- Error handling
- Proper resource cleanup

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
- **Dictionary file**: ~3-4 MB (ja_phonemes.json)
- **Dart code**: ~20 KB
- **Total package**: ~4-5 MB

### Runtime Dependencies

- `ffi: ^2.1.0` - Foreign Function Interface support
- `path: ^1.8.3` - Path manipulation utilities

### No External Dependencies

The native library is self-contained with no runtime dependencies.

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
