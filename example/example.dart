import 'package:japanese_phoneme_converter/japanese_phoneme_converter.dart';

void main() {
  print('');
  print('╔══════════════════════════════════════════════════════════╗');
  print('║  Japanese → Phoneme Converter (Dart FFI)                ║');
  print('╚══════════════════════════════════════════════════════════╝');
  print('');

  // Create converter instance
  final converter = JapanesePhonemeConverter();

  print('📚 Library version: ${converter.version}');
  print('');

  // Initialize with dictionary (adjust path as needed)
  print('🔥 Loading phoneme dictionary...');
  if (!converter.init('assets/ja_phonemes.json')) {
    print('❌ Initialization failed: ${converter.lastError}');
    print('   Make sure ja_phonemes.json is in the correct location');
    return;
  }

  final entryCount = converter.entryCount;
  print('✅ Loaded $entryCount entries');
  print('');
  print('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
  print('');

  // Test conversion examples
  final examples = [
    'こんにちは',
    '日本語',
    '東京',
    'ありがとうございます',
    '今日はいい天気ですね',
  ];

  for (final text in examples) {
    final result = converter.convert(text);
    if (result != null) {
      print('Input:    $text');
      print('Phonemes: ${result.phonemes}');
      print('Time:     ${result.processingTimeMicroseconds}μs '
          '(${result.processingTimeMilliseconds.toStringAsFixed(2)}ms)');
      print('');
    } else {
      print('❌ Failed to convert: $text');
      print('   Error: ${converter.lastError}');
      print('');
    }
  }

  print('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
  print('✨ Conversion complete!');
  print('');

  // Clean up resources
  converter.dispose();
}

