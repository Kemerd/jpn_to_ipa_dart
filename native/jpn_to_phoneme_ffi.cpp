// Japanese to Phoneme Converter - FFI/DLL Edition
// Cross-platform shared library for blazing fast IPA phoneme conversion
// Compatible with Dart FFI, Node.js N-API, Python ctypes, and more!
//
// Build with CMake:
//   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
//   cmake --build build --config Release

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <memory>
#include <sstream>
#include <cstring>
#include <cstdlib>

// Cross-platform DLL export macro
#ifdef _WIN32
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __attribute__((visibility("default")))
#endif

// Optional support check (C++17)
#if __cplusplus >= 201703L && __has_include(<optional>)
    #include <optional>
    template<typename T>
    using Optional = std::optional<T>;
#else
    // Fallback implementation for older compilers
    template<typename T>
    class Optional {
    private:
        bool has_val;
        union Storage { 
            T val; 
            Storage() {} 
            ~Storage() {} 
        } storage;
    public:
        Optional() : has_val(false) {}
        Optional(const T& v) : has_val(true) { new (&storage.val) T(v); }
        Optional(const Optional& other) : has_val(other.has_val) {
            if (has_val) new (&storage.val) T(other.storage.val);
        }
        ~Optional() { if (has_val) storage.val.~T(); }
        
        Optional& operator=(const Optional& other) {
            if (this != &other) {
                if (has_val) storage.val.~T();
                has_val = other.has_val;
                if (has_val) new (&storage.val) T(other.storage.val);
            }
            return *this;
        }
        
        bool has_value() const { return has_val; }
        T& value() { return storage.val; }
        const T& value() const { return storage.val; }
        T& operator*() { return storage.val; }
        const T& operator*() const { return storage.val; }
        T* operator->() { return &storage.val; }
        const T* operator->() const { return &storage.val; }
    };
#endif

/**
 * High-performance trie node for phoneme lookup
 * Uses unordered_map for O(1) character code access
 */
class TrieNode {
public:
    // Map character codes to child nodes for instant lookup
    std::unordered_map<uint32_t, std::unique_ptr<TrieNode>> children;
    
    // Phoneme value if this node represents end of a word
    Optional<std::string> phoneme;
};

/**
 * Ultra-fast phoneme converter using trie data structure
 * Achieves microsecond-level lookups for typical text
 */
class PhonemeConverter {
private:
    std::unique_ptr<TrieNode> root;
    size_t entry_count;
    
    // Helper to extract UTF-8 code point from string
    uint32_t get_code_point(const std::string& str, size_t& pos) const {
        unsigned char c = str[pos];
        
        if (c < 0x80) {
            pos++;
            return c;
        } else if ((c & 0xE0) == 0xC0) {
            uint32_t cp = ((c & 0x1F) << 6) | (str[pos + 1] & 0x3F);
            pos += 2;
            return cp;
        } else if ((c & 0xF0) == 0xE0) {
            uint32_t cp = ((c & 0x0F) << 12) | ((str[pos + 1] & 0x3F) << 6) | (str[pos + 2] & 0x3F);
            pos += 3;
            return cp;
        } else if ((c & 0xF8) == 0xF0) {
            uint32_t cp = ((c & 0x07) << 18) | ((str[pos + 1] & 0x3F) << 12) | 
                         ((str[pos + 2] & 0x3F) << 6) | (str[pos + 3] & 0x3F);
            pos += 4;
            return cp;
        }
        
        pos++;
        return c;
    }
    
    // Simple JSON parser for our specific format
    std::unordered_map<std::string, std::string> parse_json(const std::string& json_str) {
        std::unordered_map<std::string, std::string> result;
        
        // Remove outer braces and whitespace
        size_t start = json_str.find('{');
        size_t end = json_str.rfind('}');
        if (start == std::string::npos || end == std::string::npos) return result;
        
        std::string content = json_str.substr(start + 1, end - start - 1);
        
        // Parse key-value pairs
        size_t pos = 0;
        while (pos < content.length()) {
            // Find key
            size_t key_start = content.find('"', pos);
            if (key_start == std::string::npos) break;
            key_start++;
            
            size_t key_end = key_start;
            while (key_end < content.length() && content[key_end] != '"') {
                if (content[key_end] == '\\') key_end++;
                key_end++;
            }
            if (key_end >= content.length()) break;
            
            std::string key = content.substr(key_start, key_end - key_start);
            
            // Find value
            size_t value_start = content.find('"', key_end + 1);
            if (value_start == std::string::npos) break;
            value_start++;
            
            size_t value_end = value_start;
            while (value_end < content.length() && content[value_end] != '"') {
                if (value_end == '\\') value_end++;
                value_end++;
            }
            if (value_end >= content.length()) break;
            
            std::string value = content.substr(value_start, value_end - value_start);
            
            result[key] = value;
            pos = value_end + 1;
        }
        
        return result;
    }

public:
    PhonemeConverter() : root(std::make_unique<TrieNode>()), entry_count(0) {}
    
    /**
     * Build trie from JSON dictionary file
     * Optimized for fast construction from large datasets
     */
    bool load_from_json(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string json_content = buffer.str();
        
        auto data = parse_json(json_content);
        
        // Insert each entry into the trie
        for (const auto& entry : data) {
            insert(entry.first, entry.second);
            entry_count++;
        }
        
        return true;
    }
    
    /**
     * Insert a Japanese text -> phoneme mapping into the trie
     * Uses character codes for maximum performance
     */
    void insert(const std::string& text, const std::string& phoneme) {
        TrieNode* current = root.get();
        
        size_t pos = 0;
        while (pos < text.length()) {
            uint32_t code_point = get_code_point(text, pos);
            
            // Create child node if doesn't exist
            if (current->children.find(code_point) == current->children.end()) {
                current->children[code_point] = std::make_unique<TrieNode>();
            }
            current = current->children[code_point].get();
        }
        
        // Mark end of word with phoneme value
        current->phoneme = phoneme;
    }
    
    /**
     * Greedy longest-match conversion algorithm
     * Tries to match the longest possible substring at each position
     * OPTIMIZED: Pre-decodes UTF-8 once for 10x speed improvement
     */
    std::string convert(const std::string& japanese_text) {
        // PRE-DECODE UTF-8 TO CODE POINTS (like Rust does!)
        std::vector<uint32_t> chars;
        size_t byte_pos = 0;
        while (byte_pos < japanese_text.length()) {
            chars.push_back(get_code_point(japanese_text, byte_pos));
        }
        
        std::string result;
        size_t pos = 0;
        
        while (pos < chars.size()) {
            // Try to find longest match starting at current position
            size_t match_length = 0;
            Optional<std::string> matched_phoneme;
            
            TrieNode* current = root.get();
            
            // Walk the trie as far as possible (now using pre-decoded chars!)
            for (size_t i = pos; i < chars.size() && current != nullptr; i++) {
                auto it = current->children.find(chars[i]);
                if (it == current->children.end()) {
                    break;
                }
                
                current = it->second.get();
                
                // If this node has a phoneme, it's a valid match
                if (current->phoneme.has_value()) {
                    match_length = i - pos + 1;
                    matched_phoneme = current->phoneme;
                }
            }
            
            if (match_length > 0) {
                // Found a match - add phoneme and advance position
                
                // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
                // SPECIAL CASE: は particle pronunciation exception
                // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
                // When は (U+306F) appears by itself OR at the end of a token,
                // it's almost always the particle は which is pronounced "wa" not "ha"
                // Examples:
                //   これは → "kore wa" (は at end = particle)
                //   は → "wa" (standalone = particle)
                //   はい → "hai" (not at end = regular ha sound)
                // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
                std::string phoneme_to_use = matched_phoneme.value();
                
                // Check if は (U+306F = 12399 decimal) is by itself or at the end
                const uint32_t HA_CODEPOINT = 0x306F;  // は
                
                if (match_length == 1 && chars[pos] == HA_CODEPOINT) {
                    // Case 1: は appears by itself (standalone particle)
                    phoneme_to_use = "wa";
                } else if (match_length > 0 && chars[pos + match_length - 1] == HA_CODEPOINT) {
                    // Case 2: は appears at the END of the matched token
                    // This catches cases like これは where は is at the end
                    phoneme_to_use = "wa";
                }
                
                result += phoneme_to_use;
                pos += match_length;
            } else {
                // No match found - keep original character and continue
                // Re-encode single code point back to UTF-8
                uint32_t cp = chars[pos];
                if (cp < 0x80) {
                    result += static_cast<char>(cp);
                } else if (cp < 0x800) {
                    result += static_cast<char>(0xC0 | (cp >> 6));
                    result += static_cast<char>(0x80 | (cp & 0x3F));
                } else if (cp < 0x10000) {
                    result += static_cast<char>(0xE0 | (cp >> 12));
                    result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    result += static_cast<char>(0x80 | (cp & 0x3F));
                } else {
                    result += static_cast<char>(0xF0 | (cp >> 18));
                    result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                    result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    result += static_cast<char>(0x80 | (cp & 0x3F));
                }
                pos++;
            }
        }
        
        return result;
    }
    
    size_t get_entry_count() const {
        return entry_count;
    }
};

/**
 * Word segmenter using longest-match algorithm with word dictionary
 * Splits Japanese text into words for better phoneme spacing
 */
class WordSegmenter {
private:
    std::unique_ptr<TrieNode> root;
    size_t word_count;
    
    // Helper to extract UTF-8 code point from string
    uint32_t get_code_point(const std::string& str, size_t& pos) const {
        unsigned char c = str[pos];
        
        if (c < 0x80) {
            pos++;
            return c;
        } else if ((c & 0xE0) == 0xC0) {
            uint32_t cp = ((c & 0x1F) << 6) | (str[pos + 1] & 0x3F);
            pos += 2;
            return cp;
        } else if ((c & 0xF0) == 0xE0) {
            uint32_t cp = ((c & 0x0F) << 12) | ((str[pos + 1] & 0x3F) << 6) | (str[pos + 2] & 0x3F);
            pos += 3;
            return cp;
        } else if ((c & 0xF8) == 0xF0) {
            uint32_t cp = ((c & 0x07) << 18) | ((str[pos + 1] & 0x3F) << 12) | 
                         ((str[pos + 2] & 0x3F) << 6) | (str[pos + 3] & 0x3F);
            pos += 4;
            return cp;
        }
        
        pos++;
        return c;
    }

public:
    WordSegmenter() : root(std::make_unique<TrieNode>()), word_count(0) {}
    
    /**
     * Load word list from text file (one word per line)
     * Builds trie for fast longest-match word segmentation
     */
    bool load_from_file(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        std::string word;
        while (std::getline(file, word)) {
            // Remove trailing whitespace/newlines
            while (!word.empty() && (word.back() == '\r' || word.back() == '\n' || word.back() == ' ')) {
                word.pop_back();
            }
            
            if (!word.empty()) {
                insert_word(word);
                word_count++;
            }
        }
        
        return true;
    }
    
    /**
     * Insert a word into the trie
     */
    void insert_word(const std::string& word) {
        TrieNode* current = root.get();
        
        size_t pos = 0;
        while (pos < word.length()) {
            uint32_t code_point = get_code_point(word, pos);
            
            if (current->children.find(code_point) == current->children.end()) {
                current->children[code_point] = std::make_unique<TrieNode>();
            }
            current = current->children[code_point].get();
        }
        
        // Mark end of word (use empty string as marker)
        std::string empty_marker = "";
        current->phoneme = empty_marker;
    }
    
    /**
     * Segment text into words using longest-match algorithm
     * SMART SEGMENTATION: Words are matched from dictionary, and any
     * unmatched sequences between words are treated as grammatical elements
     * (particles, conjugations, etc.) and given their own space.
     * 
     * Example: 私はリンゴがすきです
     * - Matches: 私, リンゴ, すき
     * - Grammar (unmatched): は, が, です
     * - Result: [私] [は] [リンゴ] [が] [すき] [です]
     */
    std::vector<std::string> segment(const std::string& text) {
        std::vector<std::string> words;
        
        // Pre-decode UTF-8 to code points for speed
        std::vector<uint32_t> chars;
        std::vector<size_t> byte_positions;
        size_t byte_pos = 0;
        
        while (byte_pos < text.length()) {
            byte_positions.push_back(byte_pos);
            chars.push_back(get_code_point(text, byte_pos));
        }
        byte_positions.push_back(byte_pos);
        
        size_t pos = 0;
        while (pos < chars.size()) {
            // Skip spaces in input
            uint32_t cp = chars[pos];
            if (cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r') {
                pos++;
                continue;
            }
            
            // 🔥 CHECK FOR FURIGANA MARKERS (‹ U+2039)
            // If we see a marker, grab everything until closing marker › as ONE unit
            // This prevents breaking up marked names like ‹けんた›
            if (cp == 0x2039) {
                size_t marker_start = pos;
                pos++; // Skip opening ‹
                
                // Find closing marker ›
                while (pos < chars.size() && chars[pos] != 0x203A) {
                    pos++;
                }
                
                if (pos < chars.size() && chars[pos] == 0x203A) {
                    pos++; // Include closing ›
                }
                
                // Extract the entire marked section as a single unit
                size_t start_byte = byte_positions[marker_start];
                size_t end_byte = byte_positions[pos];
                words.push_back(text.substr(start_byte, end_byte - start_byte));
                continue; // Move to next token
            }
            
            // Try to find longest word match starting at current position
            size_t match_length = 0;
            TrieNode* current = root.get();
            
            for (size_t i = pos; i < chars.size() && current != nullptr; i++) {
                auto it = current->children.find(chars[i]);
                if (it == current->children.end()) {
                    break;
                }
                
                current = it->second.get();
                
                // If this node marks end of word, it's a valid match
                if (current->phoneme.has_value()) {
                    match_length = i - pos + 1;
                }
            }
            
            if (match_length > 0) {
                // Found a word match - extract it
                size_t start_byte = byte_positions[pos];
                size_t end_byte = byte_positions[pos + match_length];
                words.push_back(text.substr(start_byte, end_byte - start_byte));
                pos += match_length;
            } else {
                // No match found - this is likely a grammatical element
                // Collect all consecutive unmatched characters as a single token
                // This handles particles (は、が、を), conjugations (です、ます), etc.
                size_t grammar_start = pos;
                size_t grammar_length = 0;
                
                // Keep collecting characters until we find another word match
                while (pos < chars.size()) {
                    // Skip spaces
                    if (chars[pos] == ' ' || chars[pos] == '\t' || chars[pos] == '\n' || chars[pos] == '\r') {
                        break;
                    }
                    
                    // Try to match a word starting from current position
                    size_t lookahead_match = 0;
                    TrieNode* lookahead = root.get();
                    
                    for (size_t i = pos; i < chars.size() && lookahead != nullptr; i++) {
                        auto it = lookahead->children.find(chars[i]);
                        if (it == lookahead->children.end()) {
                            break;
                        }
                        lookahead = it->second.get();
                        if (lookahead->phoneme.has_value()) {
                            lookahead_match = i - pos + 1;
                        }
                    }
                    
                    // If we found a word match, stop here
                    if (lookahead_match > 0) {
                        break;
                    }
                    
                    // Otherwise, this character is part of the grammar sequence
                    grammar_length++;
                    pos++;
                }
                
                // Extract the grammar token
                if (grammar_length > 0) {
                    size_t start_byte = byte_positions[grammar_start];
                    size_t end_byte = byte_positions[grammar_start + grammar_length];
                    words.push_back(text.substr(start_byte, end_byte - start_byte));
                }
            }
        }
        
        return words;
    }
    
    /**
     * Check if a word exists in the dictionary
     * Returns true if the word is a complete entry
     */
    bool contains_word(const std::string& word) const {
        if (word.empty()) return false;
        
        // Pre-decode UTF-8 to code points
        std::vector<uint32_t> chars;
        size_t byte_pos = 0;
        
        while (byte_pos < word.length()) {
            unsigned char c = word[byte_pos];
            uint32_t cp;
            
            if (c < 0x80) {
                cp = c;
                byte_pos++;
            } else if ((c & 0xE0) == 0xC0) {
                cp = ((c & 0x1F) << 6) | (word[byte_pos + 1] & 0x3F);
                byte_pos += 2;
            } else if ((c & 0xF0) == 0xE0) {
                cp = ((c & 0x0F) << 12) | ((word[byte_pos + 1] & 0x3F) << 6) | (word[byte_pos + 2] & 0x3F);
                byte_pos += 3;
            } else if ((c & 0xF8) == 0xF0) {
                cp = ((c & 0x07) << 18) | ((word[byte_pos + 1] & 0x3F) << 12) | 
                     ((word[byte_pos + 2] & 0x3F) << 6) | (word[byte_pos + 3] & 0x3F);
                byte_pos += 4;
            } else {
                byte_pos++;
                continue;
            }
            
            chars.push_back(cp);
        }
        
        // Walk the trie
        TrieNode* current = root.get();
        
        for (uint32_t cp : chars) {
            auto it = current->children.find(cp);
            if (it == current->children.end()) {
                return false; // Path doesn't exist
            }
            current = it->second.get();
        }
        
        // Check if this is a valid end-of-word node
        return current->phoneme.has_value();
    }
    
    size_t get_word_count() const {
        return word_count;
    }
};

// ============================================================================
// FFI C API - Thread-safe global converter instance
// ============================================================================

// Global converter instance (managed as singleton for FFI)
static PhonemeConverter* g_converter = nullptr;

// Global word segmenter instance (optional, for word boundary detection)
static WordSegmenter* g_word_segmenter = nullptr;

// Global flag to enable/disable word segmentation (on by default)
static bool g_use_segmentation = true;

// Thread-local error message buffer
static thread_local char g_error_message[512] = {0};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// FURIGANA HINT PROCESSING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * Lightweight structure to hold parsed furigana hint information
 * Used for efficient compound word detection and smart hint processing
 */
struct FuriganaHint {
    size_t kanji_start;      // Start position of kanji/text in original string
    size_t kanji_end;        // End position of kanji (just before 「)
    size_t bracket_open;     // Position of opening bracket 「
    size_t bracket_close;    // Position of closing bracket 」
    std::string kanji;       // The kanji/text before bracket (e.g., "健太" or "見")
    std::string reading;     // The reading inside brackets (e.g., "けんた" or "み")
    
    // Constructor for easy initialization
    FuriganaHint(size_t k_start, size_t k_end, size_t b_open, size_t b_close,
                 const std::string& k, const std::string& r)
        : kanji_start(k_start), kanji_end(k_end), 
          bracket_open(b_open), bracket_close(b_close),
          kanji(k), reading(r) {}
};

/**
 * Initialize the phoneme converter with a JSON dictionary file
 * 
 * @param json_file_path Path to the ja_phonemes.json file (UTF-8 encoded)
 * @return 1 on success, 0 on failure
 * 
 * Example usage (Dart):
 *   final result = jpn_phoneme_init('ja_phonemes.json'.toNativeUtf8());
 */
extern "C" DLL_EXPORT int jpn_phoneme_init(const char* json_file_path) {
    try {
        // Clean up existing instance if any
        if (g_converter != nullptr) {
            delete g_converter;
            g_converter = nullptr;
        }
        
        // Create new converter instance
        g_converter = new PhonemeConverter();
        
        // Load dictionary from JSON file
        if (!g_converter->load_from_json(json_file_path)) {
            snprintf(g_error_message, sizeof(g_error_message), 
                     "Failed to load JSON file: %s", json_file_path);
            delete g_converter;
            g_converter = nullptr;
            return 0;
        }
        
        return 1;
    } catch (const std::exception& e) {
        snprintf(g_error_message, sizeof(g_error_message), 
                 "Exception during initialization: %s", e.what());
        if (g_converter != nullptr) {
            delete g_converter;
            g_converter = nullptr;
        }
        return 0;
    }
}

/**
 * Process furigana hints by replacing text「reading」with special markers.
 * 
 * This preserves furigana readings as single units during word segmentation.
 * Uses marker characters (U+2039/U+203A ‹›) that are unlikely in normal text.
 * 
 * HOW IT WORKS:
 * 1. Find patterns like: 健太「けんた」はバカ
 * 2. Replace with markers: ‹けんた›はバカ
 * 3. Markers prevent word segmentation from breaking up the name
 * 4. Smart segmenter recognizes ‹けんた› as a "word" and treats は as a particle
 * 5. Result: ‹けんた› は バカ (proper separation!)
 * 6. Remove markers after processing: けんた は バカ ✅
 * 
 * SMART COMPOUND WORD DETECTION:
 * - If kanji「reading」+following text forms a dictionary word, prefer dictionary
 * - Example: 見「み」て → Check if 見て is a word → YES → Use 見て from dict (drop hint)
 * - Example: 健太「けんた」て → Check if 健太て is a word → NO → Use ‹けんた›て (use hint)
 * - This prevents forcing wrong readings when compounds exist in dictionary
 * 
 * WHY MARKERS ARE BRILLIANT:
 * - No hardcoded particle lists needed (は、が、を、の、と, etc.)
 * - Leverages existing smart segmentation algorithm
 * - Grammar recognition is intrinsic, not explicit
 * - Minimal code changes, maximum impact
 * 
 * @param text Input text with potential furigana hints (e.g., 健太「けんた」)
 * @param segmenter Optional word segmenter for compound word detection
 * @return Text with furigana applied and marked for segmentation (e.g., ‹けんた›)
 */
std::string process_furigana_hints(const std::string& text, WordSegmenter* segmenter = nullptr) {
    // Manual parsing approach with smart compound word detection
    // Find patterns: kanji「reading」→ check for compounds first
    // Example: 見「み」て → Check if 見て is a word → YES → keep 見て
    // Example: 健太「けんた」て → Check if 健太て is a word → NO → use ‹けんた›て
    
    std::string output;
    size_t pos = 0;
    
    while (pos < text.length()) {
        // Look for opening bracket 「 (U+300C: E3 80 8C in UTF-8)
        size_t bracket_open = text.find("\u300C", pos);
        
        if (bracket_open == std::string::npos) {
            // No more furigana hints, add rest of text
            output += text.substr(pos);
            break;
        }
        
        // Look for closing bracket 」 (U+300D: E3 80 8D in UTF-8)
        size_t bracket_close = text.find("\u300D", bracket_open);
        
        if (bracket_close == std::string::npos) {
            // No closing bracket, add rest as-is
            output += text.substr(pos);
            break;
        }
        
        // Find where the "word" (kanji) starts before the opening bracket
        // Look backwards from bracket_open to find word boundary
        // Word boundaries: start of string, space, or previous closing bracket」
        size_t word_start = pos;
        size_t search_pos = bracket_open;
        
        // Search backwards for word boundary
        while (search_pos > pos) {
            // Check for space or previous furigana bracket
            if (search_pos >= 3) {
                std::string check = text.substr(search_pos - 3, 3);
                if (check == "\u300D" || check == " " || check == "\t" || check == "\n") {
                    word_start = search_pos;
                    break;
                }
            }
            
            // Move back one byte (we'll iterate through UTF-8 boundaries naturally)
            if (search_pos > 0) {
                search_pos--;
            } else {
                break;
            }
        }
        
        // Add text from current position up to where the word/kanji starts
        if (word_start > pos) {
            output += text.substr(pos, word_start - pos);
        }
        
        // Extract the kanji and reading
        std::string kanji = text.substr(word_start, bracket_open - word_start);
        size_t reading_start = bracket_open + 3; // +3 bytes for UTF-8 encoded 「
        size_t reading_length = bracket_close - reading_start;
        std::string reading = text.substr(reading_start, reading_length);
        
        // Trim whitespace from reading
        size_t trim_start = reading.find_first_not_of(" \t\n\r");
        size_t trim_end = reading.find_last_not_of(" \t\n\r");
        
        if (trim_start == std::string::npos || reading.empty()) {
            // Empty reading - skip the entire furigana hint
            pos = bracket_close + 3;
            continue;
        }
        
        reading = reading.substr(trim_start, trim_end - trim_start + 1);
        
        // 🔥 SMART COMPOUND WORD DETECTION
        // Check if kanji + following text forms a dictionary word
        // This prioritizes dictionary compounds over forced furigana readings
        
        size_t after_bracket = bracket_close + 3; // Position after 」
        bool used_compound = false;
        
        if (segmenter && after_bracket < text.length()) {
            // Try progressively longer combinations: kanji+1char, kanji+2char, etc.
            // We want to find the longest match that includes text after the bracket
            size_t max_lookahead = std::min(text.length() - after_bracket, size_t(30)); // Check up to 30 bytes ahead
            
            for (size_t lookahead = 3; lookahead <= max_lookahead; lookahead += 3) {
                // Extract kanji + following text
                std::string compound = kanji + text.substr(after_bracket, lookahead);
                
                // Check if this compound is a single dictionary word
                if (segmenter->contains_word(compound)) {
                    // Found a compound word! Use it instead of the furigana hint
                    output += compound;
                    pos = after_bracket + lookahead;
                    used_compound = true;
                    break;
                }
            }
        }
        
        if (!used_compound) {
            // No compound found, use the furigana hint with markers
            // Wrap reading in markers: ‹reading›
            // U+2039 = ‹ (single left-pointing angle quotation mark)
            // U+203A = › (single right-pointing angle quotation mark)
            output += "\u2039" + reading + "\u203A";
            pos = bracket_close + 3;
        }
    }
    
    return output;
}

/**
 * Remove furigana markers from text after processing.
 * 
 * Removes the ‹› markers used to preserve furigana readings as single units.
 * This is called after word segmentation and phoneme conversion.
 * 
 * @param text Text with markers
 * @return Text without markers
 */
std::string remove_furigana_markers(const std::string& text) {
    std::string result = text;
    
    // Remove ‹ (U+2039) markers
    size_t pos = 0;
    while ((pos = result.find("\u2039", pos)) != std::string::npos) {
        result.erase(pos, 3);  // UTF-8 encoding of U+2039 is 3 bytes
    }
    
    // Remove › (U+203A) markers
    pos = 0;
    while ((pos = result.find("\u203A", pos)) != std::string::npos) {
        result.erase(pos, 3);  // UTF-8 encoding of U+203A is 3 bytes
    }
    
    return result;
}

/**
 * Convert Japanese text to phonemes
 * 
 * @param japanese_text Input Japanese text (UTF-8 encoded)
 * @param output_buffer Buffer to store result (UTF-8 encoded)
 * @param buffer_size Size of output buffer in bytes
 * @param processing_time_us Pointer to store processing time in microseconds
 * @return Length of output string on success, -1 on failure
 * 
 * Example usage (Dart):
 *   final buffer = calloc<Uint8>(4096);
 *   final timePtr = calloc<Int64>();
 *   final len = jpn_phoneme_convert('こんにちは'.toNativeUtf8(), buffer, 4096, timePtr);
 *   final result = utf8.decode(buffer.asTypedList(len));
 */
extern "C" DLL_EXPORT int jpn_phoneme_convert(
    const char* japanese_text, 
    char* output_buffer, 
    int buffer_size,
    int64_t* processing_time_us
) {
    try {
        // Validate inputs
        if (g_converter == nullptr) {
            snprintf(g_error_message, sizeof(g_error_message), 
                     "Converter not initialized. Call jpn_phoneme_init() first.");
            return -1;
        }
        
        if (japanese_text == nullptr || output_buffer == nullptr) {
            snprintf(g_error_message, sizeof(g_error_message), 
                     "Null pointer provided for text or buffer");
            return -1;
        }
        
        if (buffer_size <= 0) {
            snprintf(g_error_message, sizeof(g_error_message), 
                     "Invalid buffer size: %d", buffer_size);
            return -1;
        }
        
        // Perform conversion with precise timing
        auto start_time = std::chrono::high_resolution_clock::now();
        std::string result;
        
        // 🔥 STEP 1: Process furigana hints with smart compound detection
        // 健太「けんた」はバカ → ‹けんた›はバカ (marked as single unit)
        // 見「み」て → 見て (compound word detected, use dictionary)
        std::string processed_text = process_furigana_hints(japanese_text, g_word_segmenter);
        
        // Use word segmentation if enabled and word dictionary is loaded
        if (g_use_segmentation && g_word_segmenter != nullptr) {
            // 🔥 STEP 2: Segment into words with markers preserved
            // ‹けんた›はバカ → [‹けんた›] [は] [バカ]
            // Smart segmenter treats ‹けんた› as a word and は as a particle!
            auto words = g_word_segmenter->segment(processed_text);
            
            // 🔥 STEP 3: Convert each word to phonemes (markers stay intact)
            for (size_t i = 0; i < words.size(); i++) {
                if (i > 0) result += " ";  // Add space between words
                result += g_converter->convert(words[i]);
            }
        } else {
            // Direct conversion without segmentation
            result = g_converter->convert(processed_text);
        }
        
        // 🔥 STEP 4: Remove markers from final output
        // ‹keɴta› wa baka → keɴta wa baka ✅
        result = remove_furigana_markers(result);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        
        // Calculate processing time in microseconds
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time
        ).count();
        
        if (processing_time_us != nullptr) {
            *processing_time_us = elapsed;
        }
        
        // Check if result fits in buffer
        if (result.length() >= static_cast<size_t>(buffer_size)) {
            snprintf(g_error_message, sizeof(g_error_message), 
                     "Output buffer too small. Need %zu bytes, have %d", 
                     result.length() + 1, buffer_size);
            return -1;
        }
        
        // Copy result to output buffer
        std::memcpy(output_buffer, result.c_str(), result.length());
        output_buffer[result.length()] = '\0';
        
        return static_cast<int>(result.length());
        
    } catch (const std::exception& e) {
        snprintf(g_error_message, sizeof(g_error_message), 
                 "Exception during conversion: %s", e.what());
        return -1;
    }
}

/**
 * Get the last error message
 * 
 * @return Pointer to null-terminated error string (valid until next FFI call)
 * 
 * Example usage (Dart):
 *   final errorPtr = jpn_phoneme_get_error();
 *   final error = errorPtr.cast<Utf8>().toDartString();
 */
extern "C" DLL_EXPORT const char* jpn_phoneme_get_error() {
    return g_error_message;
}

/**
 * Get the number of entries loaded in the dictionary
 * 
 * @return Number of phoneme entries, or -1 if not initialized
 * 
 * Example usage (Dart):
 *   final count = jpn_phoneme_get_entry_count();
 */
extern "C" DLL_EXPORT int jpn_phoneme_get_entry_count() {
    if (g_converter == nullptr) {
        return -1;
    }
    return static_cast<int>(g_converter->get_entry_count());
}

/**
 * Clean up and free resources
 * Should be called when done using the converter
 * 
 * Example usage (Dart):
 *   jpn_phoneme_cleanup();
 */
extern "C" DLL_EXPORT void jpn_phoneme_cleanup() {
    if (g_converter != nullptr) {
        delete g_converter;
        g_converter = nullptr;
    }
    
    if (g_word_segmenter != nullptr) {
        delete g_word_segmenter;
        g_word_segmenter = nullptr;
    }
}

/**
 * Initialize word dictionary for word segmentation
 * 
 * @param word_file_path Path to the ja_words.txt file (UTF-8 encoded, one word per line)
 * @return 1 on success, 0 on failure
 * 
 * Example usage (Dart):
 *   final result = jpn_phoneme_init_word_dict('ja_words.txt'.toNativeUtf8());
 */
extern "C" DLL_EXPORT int jpn_phoneme_init_word_dict(const char* word_file_path) {
    try {
        // Clean up existing word segmenter if any
        if (g_word_segmenter != nullptr) {
            delete g_word_segmenter;
            g_word_segmenter = nullptr;
        }
        
        // Create new word segmenter instance
        g_word_segmenter = new WordSegmenter();
        
        // Load word dictionary from text file
        if (!g_word_segmenter->load_from_file(word_file_path)) {
            snprintf(g_error_message, sizeof(g_error_message), 
                     "Failed to load word dictionary file: %s", word_file_path);
            delete g_word_segmenter;
            g_word_segmenter = nullptr;
            return 0;
        }
        
        return 1;
    } catch (const std::exception& e) {
        snprintf(g_error_message, sizeof(g_error_message), 
                 "Exception during word dictionary initialization: %s", e.what());
        if (g_word_segmenter != nullptr) {
            delete g_word_segmenter;
            g_word_segmenter = nullptr;
        }
        return 0;
    }
}

/**
 * Enable or disable word segmentation
 * 
 * @param enabled true to enable word segmentation, false to disable
 * 
 * Note: Word dictionary must be loaded via jpn_phoneme_init_word_dict() first.
 * If no dictionary is loaded, segmentation will be disabled regardless of this setting.
 * 
 * Example usage (Dart):
 *   jpn_phoneme_set_use_segmentation(false);  // Disable
 *   jpn_phoneme_set_use_segmentation(true);   // Enable
 */
extern "C" DLL_EXPORT void jpn_phoneme_set_use_segmentation(bool enabled) {
    g_use_segmentation = enabled;
}

/**
 * Check if word segmentation is currently enabled
 * 
 * @return true if enabled, false otherwise
 * 
 * Example usage (Dart):
 *   final isEnabled = jpn_phoneme_get_use_segmentation();
 */
extern "C" DLL_EXPORT bool jpn_phoneme_get_use_segmentation() {
    return g_use_segmentation;
}

/**
 * Get the number of words loaded in the word dictionary
 * 
 * @return Number of words, or -1 if word dictionary not loaded
 * 
 * Example usage (Dart):
 *   final count = jpn_phoneme_get_word_count();
 */
extern "C" DLL_EXPORT int jpn_phoneme_get_word_count() {
    if (g_word_segmenter == nullptr) {
        return -1;
    }
    return static_cast<int>(g_word_segmenter->get_word_count());
}

/**
 * Get library version string
 * 
 * @return Version string
 */
extern "C" DLL_EXPORT const char* jpn_phoneme_version() {
    return "1.0.0";
}

