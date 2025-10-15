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

  // Load word dictionary for segmentation
  print('🔥 Loading word dictionary for segmentation...');
  try {
    converter.loadWordDictionary('assets/ja_words.txt');
    print('✅ Loaded ${converter.wordCount} words');
    print('   💡 Word segmentation: ${converter.useSegmentation ? "ENABLED" : "DISABLED"}');
  } catch (e) {
    print('⚠️  Warning: Could not load word dictionary: $e');
    print('   Continuing without word segmentation...');
  }
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
    '私はリンゴが好きです',
  ];

  print('📝 WITH word segmentation (spaces between words):');
  print('');
  for (final text in examples) {
    final result = converter.convert(text);
    if (result != null) {
      print('Input:    $text');
      print('Phonemes: ${result.phonemes}');
      print('Time:     ${result.processingTimeMicroseconds}μs');
      print('');
    } else {
      print('❌ Failed to convert: $text');
      print('   Error: ${converter.lastError}');
      print('');
    }
  }

  // Demonstrate without segmentation
  if (converter.wordCount > 0) {
    print('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
    print('');
    print('📝 WITHOUT word segmentation (no spaces):');
    print('');
    
    // Disable segmentation
    converter.setUseSegmentation(false);
    
    for (final text in examples) {
      final result = converter.convert(text);
      if (result != null) {
        print('Input:    $text');
        print('Phonemes: ${result.phonemes}');
        print('Time:     ${result.processingTimeMicroseconds}μs');
        print('');
      }
    }
    
    // Re-enable for comparison
    converter.setUseSegmentation(true);
  }

  print('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
  print('✨ Conversion complete!');
  print('');

  // Clean up resources
  converter.dispose();
}

