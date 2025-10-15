import 'package:test/test.dart';
import 'package:japanese_phoneme_converter/japanese_phoneme_converter.dart';

void main() {
  group('JapanesePhonemeConverter', () {
    late JapanesePhonemeConverter converter;

    setUp(() {
      converter = JapanesePhonemeConverter();
    });

    tearDown(() {
      converter.dispose();
    });

    test('should load library and get version', () {
      expect(converter.version, isNotEmpty);
      expect(converter.version, contains('.'));
    });

    test('should initialize with valid dictionary', () {
      // Adjust path based on your test setup
      final success = converter.init('assets/ja_phonemes.json');
      expect(success, isTrue);
      expect(converter.isInitialized, isTrue);
      expect(converter.entryCount, greaterThan(0));
    });

    test('should throw exception when converting without initialization', () {
      expect(
        () => converter.convertOrThrow('こんにちは'),
        throwsA(isA<PhonemeException>()),
      );
    });

    test('should convert Japanese text to phonemes', () {
      converter.init('assets/ja_phonemes.json');

      final result = converter.convert('こんにちは');
      expect(result, isNotNull);
      expect(result!.phonemes, isNotEmpty);
      expect(result.processingTimeMicroseconds, greaterThan(0));
    });

    test('should provide detailed conversion result', () {
      converter.init('assets/ja_phonemes.json');

      final result = converter.convertOrThrow('日本語');
      expect(result.phonemes, isNotEmpty);
      expect(result.processingTimeMicroseconds, greaterThan(0));
      expect(result.processingTimeMilliseconds, greaterThan(0));
    });

    test('should handle empty string', () {
      converter.init('assets/ja_phonemes.json');

      final result = converter.convert('');
      expect(result, isNotNull);
    });

    test('should be thread-safe after initialization', () async {
      converter.init('assets/ja_phonemes.json');

      // Run multiple conversions concurrently
      final futures = List.generate(
        100,
        (_) => Future(() => converter.convert('こんにちは')),
      );

      final results = await Future.wait(futures);
      expect(results.every((r) => r != null), isTrue);
      expect(results.every((r) => r!.phonemes.isNotEmpty), isTrue);
    });

    test('should not allow use after dispose', () {
      converter.init('assets/ja_phonemes.json');
      converter.dispose();

      expect(converter.isDisposed, isTrue);
      expect(converter.isInitialized, isFalse);
      expect(() => converter.convert('test'), throwsA(isA<PhonemeException>()));
    });

    test('should handle multiple dispose calls safely', () {
      converter.dispose();
      converter.dispose(); // Should not throw
      expect(converter.isDisposed, isTrue);
    });

    test('ConversionResult equality', () {
      final result1 = ConversionResult(phonemes: 'test', processingTimeMicroseconds: 100);
      final result2 = ConversionResult(phonemes: 'test', processingTimeMicroseconds: 100);
      final result3 = ConversionResult(phonemes: 'different', processingTimeMicroseconds: 100);

      expect(result1, equals(result2));
      expect(result1, isNot(equals(result3)));
    });

    test('ConversionResult toString', () {
      final result = ConversionResult(phonemes: 'test', processingTimeMicroseconds: 100);
      expect(result.toString(), contains('test'));
      expect(result.toString(), contains('100'));
    });

    test('PhonemeException message', () {
      final exception = PhonemeException('test error');
      expect(exception.toString(), contains('test error'));
    });

    group('Word Segmentation', () {
      test('should load word dictionary successfully', () {
        converter.init('assets/ja_phonemes.json');
        
        // Load word dictionary
        converter.loadWordDictionary('assets/ja_words.txt');
        
        expect(converter.wordCount, greaterThan(0));
      });

      test('should throw exception when word dictionary file not found', () {
        converter.init('assets/ja_phonemes.json');
        
        expect(
          () => converter.loadWordDictionary('nonexistent.txt'),
          throwsA(isA<PhonemeException>()),
        );
      });

      test('should enable/disable word segmentation', () {
        converter.init('assets/ja_phonemes.json');
        converter.loadWordDictionary('assets/ja_words.txt');
        
        // Default should be enabled
        expect(converter.useSegmentation, isTrue);
        
        // Disable segmentation
        converter.setUseSegmentation(false);
        expect(converter.useSegmentation, isFalse);
        
        // Re-enable
        converter.setUseSegmentation(true);
        expect(converter.useSegmentation, isTrue);
      });

      test('should add spaces when segmentation is enabled', () {
        converter.init('assets/ja_phonemes.json');
        converter.loadWordDictionary('assets/ja_words.txt');
        
        // Enable segmentation
        converter.setUseSegmentation(true);
        final withSpaces = converter.convert('私はリンゴが好きです');
        
        // Disable segmentation
        converter.setUseSegmentation(false);
        final withoutSpaces = converter.convert('私はリンゴが好きです');
        
        expect(withSpaces, isNotNull);
        expect(withoutSpaces, isNotNull);
        expect(withSpaces!.phonemes, contains(' '));
        expect(withoutSpaces!.phonemes, isNot(contains(' ')));
      });

      test('should work without word dictionary loaded', () {
        converter.init('assets/ja_phonemes.json');
        
        // Don't load word dictionary - segmentation should be disabled
        expect(converter.wordCount, equals(-1));
        
        final result = converter.convert('こんにちは');
        expect(result, isNotNull);
        expect(result!.phonemes, isNotEmpty);
      });

      test('should handle word segmentation toggle without reload', () {
        converter.init('assets/ja_phonemes.json');
        converter.loadWordDictionary('assets/ja_words.txt');
        
        final testText = '今日はいい天気ですね';
        
        // With segmentation
        converter.setUseSegmentation(true);
        final result1 = converter.convert(testText);
        
        // Without segmentation
        converter.setUseSegmentation(false);
        final result2 = converter.convert(testText);
        
        // With segmentation again
        converter.setUseSegmentation(true);
        final result3 = converter.convert(testText);
        
        expect(result1, isNotNull);
        expect(result2, isNotNull);
        expect(result3, isNotNull);
        
        // Results with segmentation should match
        expect(result1!.phonemes, equals(result3!.phonemes));
        
        // Results should differ based on segmentation
        expect(result1.phonemes, isNot(equals(result2!.phonemes)));
      });
    });
  });
}

