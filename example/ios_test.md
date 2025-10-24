# iOS Testing Guide for Japanese Phoneme Converter

## Prerequisites

1. **macOS with Xcode installed**
2. **Flutter SDK**
3. **CocoaPods** (`sudo gem install cocoapods`)

## Testing Steps

### 1. Build the iOS Framework

```bash
cd dart_ffi/ios
chmod +x build.sh
./build.sh
```

This will create the XCFramework with all necessary architectures.

### 2. Create or Use Example Flutter App

If you don't have an example app:

```bash
cd dart_ffi
flutter create example
cd example
```

### 3. Add the Plugin Dependency

Edit `pubspec.yaml`:

```yaml
dependencies:
  flutter:
    sdk: flutter
  japanese_phoneme_converter:
    path: ../
```

### 4. Update iOS Platform Version

Edit `example/ios/Podfile`:

```ruby
platform :ios, '11.0'
```

### 5. Install Dependencies

```bash
cd example/ios
pod install
cd ..
```

### 6. Test Code

Create `example/lib/main.dart`:

```dart
import 'package:flutter/material.dart';
import 'package:japanese_phoneme_converter/japanese_phoneme_converter.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final converter = JapanesePhonemeConverter();
  String _status = 'Not initialized';
  String _result = '';

  @override
  void initState() {
    super.initState();
    _initConverter();
  }

  Future<void> _initConverter() async {
    try {
      // Initialize converter with bundled asset
      bool success = await converter.init();
      setState(() {
        _status = success ? 'Ready' : 'Failed: ${converter.lastError}';
      });
    } catch (e) {
      setState(() {
        _status = 'Error: $e';
      });
    }
  }

  void _testConversion() {
    if (!converter.isInitialized) {
      setState(() {
        _result = 'Converter not ready!';
      });
      return;
    }

    final testTexts = ['こんにちは', '日本語', 'ありがとう'];
    final results = <String>[];

    for (final text in testTexts) {
      final result = converter.convert(text);
      if (result != null) {
        results.add('$text → ${result.phonemes} (${result.processingTimeMicroseconds}μs)');
      } else {
        results.add('$text → Error: ${converter.lastError}');
      }
    }

    setState(() {
      _result = results.join('\n');
    });
  }

  @override
  void dispose() {
    converter.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Text('Japanese Phoneme Converter Test'),
        ),
        body: Center(
          child: Padding(
            padding: EdgeInsets.all(20),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text('Status: $_status'),
                SizedBox(height: 20),
                ElevatedButton(
                  onPressed: converter.isInitialized ? _testConversion : null,
                  child: Text('Test Conversion'),
                ),
                SizedBox(height: 20),
                Text(
                  _result,
                  style: TextStyle(fontFamily: 'monospace'),
                  textAlign: TextAlign.center,
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
```

### 7. Run on iOS

```bash
# List available devices
flutter devices

# Run on iOS simulator
flutter run -d iPhone

# Or run on physical device
flutter run -d <device-id>
```

## Troubleshooting

### "Phoneme converter not ready" Error

This usually means the FFI symbols aren't being linked properly. Check:

1. **Build logs**: Look for any linking errors
2. **Symbol visibility**: Run `nm -gU` on the built framework
3. **Console logs**: Check Console.app for runtime errors

### Pod Install Fails

1. Clean pods: `cd ios && pod deintegrate && pod install`
2. Update repo: `pod repo update`
3. Clear caches: `rm -rf ~/Library/Caches/CocoaPods`

### Symbols Not Found

Ensure the podspec includes all necessary flags:
- `OTHER_LDFLAGS` includes `-ObjC -all_load`
- `GCC_SYMBOLS_PRIVATE_EXTERN` is set to `NO`
- The .mm file properly declares extern "C" functions

### Debug Tips

1. **Enable FFI logging**: The C++ code includes debug logging
2. **Check asset loading**: Ensure `japanese.trie` is bundled
3. **Verify initialization**: Check return value and error messages

## Performance Testing

Once working, test performance with larger texts:

```dart
final longText = '長い日本語のテキスト' * 100;
final result = converter.convert(longText);
print('Converted ${longText.length} chars in ${result?.processingTimeMicroseconds}μs');
```

Expected performance: ~1-5 microseconds per character on modern iOS devices.
