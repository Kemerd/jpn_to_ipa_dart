## 2.0.0 - Smart Furigana Hints (October 16, 2025)

### ✨ NEW FEATURES
- **Furigana Hint Support**: Use `「」` brackets to provide pronunciation hints for names/uncommon words
- **Smart Compound Word Detection**: Automatically detects and prioritizes dictionary compound words
- **Marker-Based Tokenization**: Preserves furigana readings as single units during segmentation
- **`FuriganaHint` struct**: Clean data structure for efficient hint processing
- **`WordSegmenter::contains_word()`**: Dictionary lookup method for compound detection

### 🔥 ENHANCED
- **Furigana processing**: Now checks if kanji+following text forms dictionary word before using hint
- **Performance**: <10% overhead for furigana processing (~5-10μs per sentence)
- **Backwards compatibility**: Text without furigana hints works exactly as before

### 📚 DOCUMENTATION
- Added comprehensive furigana section to README
- Documented smart compound detection algorithm
- Added usage examples for various scenarios

### 🎯 EXAMPLES
- `見「み」て` → Uses `見て` from dictionary (compound detected)
- `健太「けんた」さん` → Uses furigana hint (no compound found)
- `健太「けんた」はバカ` → Proper particle separation: `keɴta ha baka`

---

## 1.0.0

- Initial release
- Cross-platform support for Windows, Linux, macOS, Android, iOS
- Microsecond-level conversion performance using native FFI
- Comprehensive API with error handling and performance metrics
- Thread-safe conversion operations
- Included phoneme dictionary (ja_phonemes.json)
- Word segmentation with ja_words.txt (147k+ words)
- Full test coverage
- Complete documentation and examples

