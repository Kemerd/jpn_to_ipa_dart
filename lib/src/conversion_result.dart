/// Result of a Japanese to phoneme conversion operation.
///
/// Contains the converted phoneme string and performance metrics.
class ConversionResult {
  /// The converted IPA phoneme representation of the input text.
  final String phonemes;

  /// Time taken for the conversion in microseconds.
  final int processingTimeMicroseconds;

  /// Time taken for the conversion in milliseconds.
  double get processingTimeMilliseconds => processingTimeMicroseconds / 1000.0;

  /// Creates a conversion result.
  const ConversionResult({
    required this.phonemes,
    required this.processingTimeMicroseconds,
  });

  @override
  String toString() {
    return 'ConversionResult(phonemes: "$phonemes", time: ${processingTimeMicroseconds}μs)';
  }

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) return true;
    return other is ConversionResult &&
        other.phonemes == phonemes &&
        other.processingTimeMicroseconds == processingTimeMicroseconds;
  }

  @override
  int get hashCode => Object.hash(phonemes, processingTimeMicroseconds);
}

