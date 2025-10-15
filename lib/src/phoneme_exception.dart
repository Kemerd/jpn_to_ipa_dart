/// Exception thrown when phoneme conversion operations fail.
class PhonemeException implements Exception {
  /// The error message describing what went wrong.
  final String message;

  /// Creates a phoneme exception with the given error message.
  const PhonemeException(this.message);

  @override
  String toString() => 'PhonemeException: $message';
}

