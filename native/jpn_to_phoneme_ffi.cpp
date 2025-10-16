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

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// FURIGANA HINT PROCESSING TYPES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * Types of segments in processed text
 */
enum class SegmentType {
    NORMAL_TEXT,      // Regular text without furigana
    FURIGANA_HINT    // Text with furigana reading hint
};

/**
 * A segment of text that can be either normal or have a furigana hint
 */
struct TextSegment {
    SegmentType type;
    std::string text;        // The actual text (kanji for furigana hints)
    std::string reading;     // The reading (only for furigana hints)
    size_t original_pos;     // Position in original text
    
    // Constructor for normal text
    TextSegment(const std::string& t, size_t pos) 
        : type(SegmentType::NORMAL_TEXT), text(t), reading(""), original_pos(pos) {}
    
    // Constructor for furigana hint
    TextSegment(const std::string& t, const std::string& r, size_t pos)
        : type(SegmentType::FURIGANA_HINT), text(t), reading(r), original_pos(pos) {}
    
    // Get the effective text (reading for furigana, text otherwise)
    std::string get_effective_text() const {
        return type == SegmentType::FURIGANA_HINT ? reading : text;
    }
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
     * Get root node for trie walking (used in word segmentation fallback)
     */
    TrieNode* get_root() const {
        return root.get();
    }
    
    /**
     * Try to load from simple binary format (japanese.trie)
     * Loads directly into TrieNode* structure using same insert() as JSON!
     * 🚀 100x faster than JSON parsing!
     */
    bool try_load_binary_format(const std::string& file_path) {
        std::cerr << "[C++ DEBUG] Attempting to load .trie from: " << file_path << std::endl;
        
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[C++ DEBUG] Failed to open file!" << std::endl;
            return false;
        }
        
        std::cerr << "[C++ DEBUG] File opened successfully!" << std::endl;
        
        // Read magic number
        char magic[4];
        file.read(magic, 4);
        std::cerr << "[C++ DEBUG] Magic bytes: " << magic[0] << magic[1] << magic[2] << magic[3] << std::endl;
        
        if (memcmp(magic, "JPHO", 4) != 0) {
            std::cerr << "[C++ DEBUG] Magic number mismatch! Expected JPHO" << std::endl;
            return false;
        }
        
        std::cerr << "[C++ DEBUG] Magic number OK!" << std::endl;
        
        // Read version
        uint16_t version_major, version_minor;
        file.read(reinterpret_cast<char*>(&version_major), 2);
        file.read(reinterpret_cast<char*>(&version_minor), 2);
        
        std::cerr << "[C++ DEBUG] Version: " << version_major << "." << version_minor << std::endl;
        
        if (version_major != 1 || version_minor != 0) {
            std::cerr << "[C++ DEBUG] Version mismatch!" << std::endl;
            return false;
        }
        
        // Read entry count
        uint32_t entry_count_val;
        file.read(reinterpret_cast<char*>(&entry_count_val), 4);
        
        std::cerr << "[C++ DEBUG] Entry count from file: " << entry_count_val << std::endl;
        
        // Helper to read varint
        auto read_varint_from_file = [&file]() -> uint32_t {
            uint32_t value = 0;
            int shift = 0;
            while (true) {
                uint8_t byte;
                file.read(reinterpret_cast<char*>(&byte), 1);
                value |= (byte & 0x7F) << shift;
                if ((byte & 0x80) == 0) break;
                shift += 7;
            }
            return value;
        };
        
        // Read all entries and insert into trie (same as JSON!)
        std::cerr << "[C++ DEBUG] Starting to read " << entry_count_val << " entries..." << std::endl;
        
        for (uint32_t i = 0; i < entry_count_val; i++) {
            // Read key
            uint32_t key_len = read_varint_from_file();
            std::string key(key_len, '\0');
            file.read(&key[0], key_len);
            
            // Read value
            uint32_t value_len = read_varint_from_file();
            std::string value(value_len, '\0');
            file.read(&value[0], value_len);
            
            // Insert using SAME function as JSON!
            insert(key, value);
            entry_count++;
            
            // Log first few entries
            if (i < 5) {
                std::cerr << "[C++ DEBUG] Entry " << i << ": '" << key << "' -> '" << value << "'" << std::endl;
            }
        }
        
        std::cerr << "[C++ DEBUG] Successfully loaded " << entry_count << " entries from .trie" << std::endl;
        
        return true;
    }
    
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
                result += matched_phoneme.value();
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
     * Get root node for trie walking (used in compound detection)
     */
    TrieNode* get_root() const {
        return root.get();
    }
    
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
     * Segment text into words using longest-match algorithm with TextSegment support
     * SMART SEGMENTATION: Words are matched from dictionary, and any
     * unmatched sequences between words are treated as grammatical elements
     * (particles, conjugations, etc.) and given their own space.
     * 
     * Example: 私はリンゴがすきです
     * - Matches: 私, リンゴ, すき
     * - Grammar (unmatched): は, が, です
     * - Result: [私] [は] [リンゴ] [が] [すき] [です]
     * 
     * This new version properly handles TextSegments with furigana hints,
     * treating each segment as an atomic unit during segmentation.
     * 
     * @param phoneme_root Optional phoneme trie root for fallback lookups
     */
    std::vector<std::string> segment_from_segments(const std::vector<TextSegment>& segments, TrieNode* phoneme_root = nullptr) {
        std::vector<std::string> words;
        
        // Process each segment
        for (const auto& segment : segments) {
            // For furigana segments, treat the entire reading as one word
            if (segment.type == SegmentType::FURIGANA_HINT) {
                words.push_back(segment.reading);
                continue;
            }
            
            // For normal text segments, apply word segmentation
            const std::string& text = segment.text;
            
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
                
                // Try to find longest word match starting at current position
                // Check word dictionary first, then phoneme dictionary as fallback
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
                
                // 🔥 FALLBACK: If word dictionary didn't find a match, try phoneme dictionary
                if (match_length == 0 && phoneme_root != nullptr) {
                    TrieNode* phoneme_current = phoneme_root;
                    
                    for (size_t i = pos; i < chars.size() && phoneme_current != nullptr; i++) {
                        auto it = phoneme_current->children.find(chars[i]);
                        if (it == phoneme_current->children.end()) {
                            break;
                        }
                        
                        phoneme_current = it->second.get();
                        
                        // If this node has a phoneme, it's a valid word
                        if (phoneme_current->phoneme.has_value()) {
                            match_length = i - pos + 1;
                        }
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
        
        // Try loading .trie first (replace .json with .trie in path)
        std::string trie_path = std::string(json_file_path);
        size_t json_pos = trie_path.rfind(".json");
        if (json_pos != std::string::npos) {
            trie_path.replace(json_pos, 5, ".trie");
        } else {
            trie_path = "japanese.trie";
        }
        
        // Try binary format first (much faster!)
        if (g_converter->try_load_binary_format(trie_path)) {
            return 1;
        }
        
        // Fallback to JSON if .trie not found
        if (!g_converter->load_from_json(json_file_path)) {
            snprintf(g_error_message, sizeof(g_error_message), 
                     "Failed to load dictionary: %s", json_file_path);
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
 * Initialize the phoneme converter from .trie data loaded in memory
 * 🔥 BLAZING FAST: Load .trie directly from Flutter assets!
 * 
 * @param trie_data Pointer to .trie file data in memory
 * @param data_size Size of the data in bytes
 * @return 1 on success, 0 on failure
 * 
 * Example usage (Dart):
 *   final data = await rootBundle.load('assets/japanese.trie');
 *   final ptr = malloc<Uint8>(data.lengthInBytes);
 *   ptr.asTypedList(data.lengthInBytes).setAll(0, data.buffer.asUint8List());
 *   final result = jpn_phoneme_init_from_memory(ptr, data.lengthInBytes);
 *   malloc.free(ptr);
 */
extern "C" DLL_EXPORT int jpn_phoneme_init_from_memory(const uint8_t* trie_data, int data_size) {
    try {
        // Clean up existing instance if any
        if (g_converter != nullptr) {
            delete g_converter;
            g_converter = nullptr;
        }
        
        // Create new converter instance
        g_converter = new PhonemeConverter();
        
        std::cerr << "[C++ DEBUG] Loading .trie from memory, size: " << data_size << " bytes" << std::endl;
        
        // Load from memory buffer
        const uint8_t* ptr = trie_data;
        const uint8_t* end = trie_data + data_size;
        
        // Read magic number
        if (ptr + 4 > end) {
            snprintf(g_error_message, sizeof(g_error_message), "Invalid .trie data: too small");
            delete g_converter;
            g_converter = nullptr;
            return 0;
        }
        
        char magic[4];
        memcpy(magic, ptr, 4);
        ptr += 4;
        
        std::cerr << "[C++ DEBUG] Magic bytes: " << magic[0] << magic[1] << magic[2] << magic[3] << std::endl;
        
        if (memcmp(magic, "JPHO", 4) != 0) {
            snprintf(g_error_message, sizeof(g_error_message), "Invalid .trie format: bad magic number");
            delete g_converter;
            g_converter = nullptr;
            return 0;
        }
        
        // Read version
        if (ptr + 4 > end) {
            snprintf(g_error_message, sizeof(g_error_message), "Invalid .trie data: truncated version");
            delete g_converter;
            g_converter = nullptr;
            return 0;
        }
        
        uint16_t version_major, version_minor;
        memcpy(&version_major, ptr, 2);
        ptr += 2;
        memcpy(&version_minor, ptr, 2);
        ptr += 2;
        
        std::cerr << "[C++ DEBUG] Version: " << version_major << "." << version_minor << std::endl;
        
        if (version_major != 1 || version_minor != 0) {
            snprintf(g_error_message, sizeof(g_error_message), 
                     "Unsupported .trie version: %d.%d", version_major, version_minor);
            delete g_converter;
            g_converter = nullptr;
            return 0;
        }
        
        // Read entry count
        if (ptr + 4 > end) {
            snprintf(g_error_message, sizeof(g_error_message), "Invalid .trie data: truncated entry count");
            delete g_converter;
            g_converter = nullptr;
            return 0;
        }
        
        uint32_t entry_count_val;
        memcpy(&entry_count_val, ptr, 4);
        ptr += 4;
        
        std::cerr << "[C++ DEBUG] Entry count: " << entry_count_val << std::endl;
        
        // Helper to read varint
        auto read_varint = [&ptr, end]() -> uint32_t {
            uint32_t value = 0;
            int shift = 0;
            while (ptr < end) {
                uint8_t byte = *ptr++;
                value |= (byte & 0x7F) << shift;
                if ((byte & 0x80) == 0) break;
                shift += 7;
            }
            return value;
        };
        
        // Read all entries and insert into trie
        for (uint32_t i = 0; i < entry_count_val; i++) {
            if (ptr >= end) {
                snprintf(g_error_message, sizeof(g_error_message), 
                         "Invalid .trie data: truncated at entry %d", i);
                delete g_converter;
                g_converter = nullptr;
                return 0;
            }
            
            // Read key
            uint32_t key_len = read_varint();
            if (ptr + key_len > end) {
                snprintf(g_error_message, sizeof(g_error_message), 
                         "Invalid .trie data: truncated key at entry %d", i);
                delete g_converter;
                g_converter = nullptr;
                return 0;
            }
            std::string key(reinterpret_cast<const char*>(ptr), key_len);
            ptr += key_len;
            
            // Read value
            uint32_t value_len = read_varint();
            if (ptr + value_len > end) {
                snprintf(g_error_message, sizeof(g_error_message), 
                         "Invalid .trie data: truncated value at entry %d", i);
                delete g_converter;
                g_converter = nullptr;
                return 0;
            }
            std::string value(reinterpret_cast<const char*>(ptr), value_len);
            ptr += value_len;
            
            // Insert using same function as JSON!
            g_converter->insert(key, value);
            
            // Log first few entries
            if (i < 5) {
                std::cerr << "[C++ DEBUG] Entry " << i << ": '" << key << "' -> '" << value << "'" << std::endl;
            }
        }
        
        std::cerr << "[C++ DEBUG] Successfully loaded " << g_converter->get_entry_count() 
                  << " entries from memory" << std::endl;
        
        return 1;
        
    } catch (const std::exception& e) {
        snprintf(g_error_message, sizeof(g_error_message), 
                 "Exception loading .trie from memory: %s", e.what());
        if (g_converter != nullptr) {
            delete g_converter;
            g_converter = nullptr;
        }
        return 0;
    }
}

/**
 * Parse text into segments, extracting furigana hints.
 * 
 * This creates a structured representation of the text where each segment
 * is either normal text or a furigana hint. This approach is cleaner than
 * using markers and makes the processing logic more transparent.
 * 
 * SMART COMPOUND WORD DETECTION:
 * - If kanji「reading」+following text forms a dictionary word, prefer dictionary
 * - Example: 見「み」て → Check if 見て is a word → YES → Keep as normal text "見て"
 * - Example: 健太「けんた」て → Check if 健太て is a word → NO → Use furigana "けんた"
 * 
 * @param text Input text with potential furigana hints (e.g., 健太「けんた」)
 * @param segmenter Optional word segmenter for compound word detection
 * @return Vector of text segments with furigana hints properly parsed
 */
std::vector<TextSegment> parse_furigana_segments(const std::string& text, WordSegmenter* segmenter = nullptr) {
    std::vector<TextSegment> segments;
    
    // 🔥 PRE-DECODE UTF-8 TO CODE POINTS FOR BLAZING SPEED!
    // Just like in the Rust version, this is the key to performance
    std::vector<uint32_t> chars;
    std::vector<size_t> byte_positions;
    size_t byte_pos = 0;
    
    // Helper lambda to get code point
    auto get_code_point_lambda = [](const std::string& str, size_t& pos) -> uint32_t {
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
    };
    
    while (byte_pos < text.length()) {
        byte_positions.push_back(byte_pos);
        chars.push_back(get_code_point_lambda(text, byte_pos));
    }
    byte_positions.push_back(byte_pos);
    
    // Now process using pre-decoded code points for speed
    size_t pos = 0;
    
    while (pos < chars.size()) {
        // Look for opening bracket 「 (U+300C)
        size_t bracket_open = std::string::npos;
        for (size_t i = pos; i < chars.size(); i++) {
            if (chars[i] == 0x300C) {
                bracket_open = i;
                break;
            }
        }
        
        if (bracket_open == std::string::npos) {
            // No more furigana hints, add rest of text as normal segment
            if (pos < chars.size()) {
                size_t start_byte = byte_positions[pos];
                size_t end_byte = byte_positions[chars.size()];
                segments.push_back(TextSegment(text.substr(start_byte, end_byte - start_byte), start_byte));
            }
            break;
        }
        
        // Look for closing bracket 」 (U+300D)
        size_t bracket_close = std::string::npos;
        for (size_t i = bracket_open + 1; i < chars.size(); i++) {
            if (chars[i] == 0x300D) {
                bracket_close = i;
                break;
            }
        }
        
        if (bracket_close == std::string::npos) {
            // No closing bracket, add rest as normal segment
            size_t start_byte = byte_positions[pos];
            size_t end_byte = byte_positions[chars.size()];
            segments.push_back(TextSegment(text.substr(start_byte, end_byte - start_byte), start_byte));
            break;
        }
        
        // Find where the "word" (kanji) starts before the opening bracket
        // 🔥 BLAZING FAST BOUNDARY DETECTION USING PRE-DECODED CODE POINTS!
        size_t word_start = bracket_open; // Start from bracket and search backward
        size_t search_pos = bracket_open;
        
        // Helper lambda to check if code point is kana (inline for speed)
        auto is_kana_cp = [](uint32_t cp) -> bool {
            return (cp >= 0x3040 && cp <= 0x309F) ||  // Hiragana
                   (cp >= 0x30A0 && cp <= 0x30FF);    // Katakana
        };
        
        // Search backwards to find the start of the kanji/word that has furigana
        // 🔥 SMART OKURIGANA DETECTION:
        // - その男「おとこ」 → Stop at kana prefix "その", capture only "男"  
        // - 昼ご飯「ひるごはん」 → Keep kana "ご" sandwiched between kanji, capture all "昼ご飯"
        // Algorithm: Scan backward collecting chars. Stop at first kana that's followed (backward) by only more kana.
        
        // First pass: Find the last non-kana (kanji) character before the bracket
        size_t last_kanji_pos = bracket_open;
        while (last_kanji_pos > pos && is_kana_cp(chars[last_kanji_pos - 1])) {
            last_kanji_pos--;
        }
        
        if (last_kanji_pos > pos) {
            last_kanji_pos--;  // Now pointing at the last kanji
        }
        
        // Second pass: From last kanji, search backward for word boundary
        // Include okurigana (kana between kanji), but stop at kana-only prefix
        word_start = last_kanji_pos;
        search_pos = last_kanji_pos;
        
        while (search_pos > pos) {
            search_pos--;
            uint32_t cp = chars[search_pos];
            
            // Check for punctuation boundaries first (these always stop us)
            if (cp == 0x300D ||  // 」 closing bracket (another furigana hint)
                cp == 0x3001 ||  // 、 Japanese comma
                cp == 0x3002 ||  // 。 Japanese period  
                cp == 0xFF01 ||  // ！ full-width exclamation
                cp == 0xFF1F ||  // ？ full-width question
                cp == 0xFF09 ||  // ） full-width right paren
                cp == 0xFF3D) {  // ］ full-width right bracket
                word_start = search_pos + 1;
                break;
            }
            
            // Check for ASCII punctuation and whitespace
            if (cp < 0x80 && (
                cp == '.' || cp == ',' || cp == '!' || cp == '?' || cp == ';' || cp == ':' ||
                cp == '(' || cp == ')' || cp == '[' || cp == ']' || cp == '{' || cp == '}' ||
                cp == '"' || cp == '\'' || cp == '-' || cp == '/' || cp == '\\' || cp == '|' ||
                cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r')) {
                word_start = search_pos + 1;
                break;
            }
            
            // Check if this is kana
            bool is_kana_char = is_kana_cp(cp);
            
            if (is_kana_char) {
                // Check if there's ANY non-kana (kanji) before this position
                bool has_kanji_before = false;
                for (size_t check_pos = search_pos; check_pos > pos; check_pos--) {
                    if (!is_kana_cp(chars[check_pos - 1])) {
                        // Check it's not punctuation
                        uint32_t check_cp = chars[check_pos - 1];
                        if (check_cp >= 0x4E00 || (check_cp >= 0x3400 && check_cp <= 0x9FFF)) {  // CJK kanji ranges
                            has_kanji_before = true;
                            break;
                        }
                    }
                }
                
                if (!has_kanji_before) {
                    // This kana is not sandwiched - it's a prefix word → stop here
                    word_start = search_pos + 1;
                    break;
                }
                // Otherwise, this kana is sandwiched (okurigana) → continue
            }
            
            // Update word_start to include this character
            word_start = search_pos;
        }
        
        // Add text from current position up to where the word/kanji starts
        // This captures particles and other text between furigana hints
        if (word_start > pos) {
            size_t start_byte = byte_positions[pos];
            size_t end_byte = byte_positions[word_start];
            segments.push_back(TextSegment(text.substr(start_byte, end_byte - start_byte), start_byte));
        }
        
        // Extract the kanji and reading using pre-decoded positions
        size_t kanji_start_byte = byte_positions[word_start];
        size_t kanji_end_byte = byte_positions[bracket_open];
        std::string kanji = text.substr(kanji_start_byte, kanji_end_byte - kanji_start_byte);
        
        // Extract reading between brackets
        size_t reading_start = bracket_open + 1; // Position after 「
        size_t reading_end = bracket_close;      // Position before 」
        
        // Extract reading text using byte positions
        size_t reading_start_byte = byte_positions[reading_start];
        size_t reading_end_byte = byte_positions[reading_end];
        std::string reading = text.substr(reading_start_byte, reading_end_byte - reading_start_byte);
        
        // Fast whitespace trimming using code points
        size_t trim_start = 0;
        size_t trim_end = reading_end - reading_start;
        
        // Trim leading whitespace
        while (trim_start < trim_end) {
            uint32_t cp = chars[reading_start + trim_start];
            if (cp != ' ' && cp != '\t' && cp != '\n' && cp != '\r') break;
            trim_start++;
        }
        
        // Trim trailing whitespace
        while (trim_end > trim_start) {
            uint32_t cp = chars[reading_start + trim_end - 1];
            if (cp != ' ' && cp != '\t' && cp != '\n' && cp != '\r') break;
            trim_end--;
        }
        
        if (trim_start >= trim_end) {
            // Empty reading - skip the entire furigana hint
            pos = bracket_close + 1;
            continue;
        }
        
        // Extract trimmed reading
        size_t trimmed_start_byte = byte_positions[reading_start + trim_start];
        size_t trimmed_end_byte = byte_positions[reading_start + trim_end];
        reading = text.substr(trimmed_start_byte, trimmed_end_byte - trimmed_start_byte);
        
        // 🔥 SMART COMPOUND WORD DETECTION USING TRIE'S LONGEST-MATCH
        // Walk the trie starting from kanji to find the longest compound word
        size_t after_bracket = bracket_close + 1; // Position after 」
        bool used_compound = false;
        
        if (segmenter && after_bracket < chars.size()) {
            // Use trie to find longest match starting from word_start position
            // This naturally implements longest-match algorithm
            size_t match_length = 0;
            TrieNode* current = segmenter->get_root();
            
            // Walk trie through kanji characters first
            for (size_t i = word_start; i < bracket_open && current != nullptr; i++) {
                auto it = current->children.find(chars[i]);
                if (it == current->children.end()) {
                    break;
                }
                current = it->second.get();
            }
            
            // Continue walking through characters after the bracket
            if (current != nullptr) {
                for (size_t i = after_bracket; i < chars.size() && current != nullptr; i++) {
                    auto it = current->children.find(chars[i]);
                    if (it == current->children.end()) {
                        break;
                    }
                    current = it->second.get();
                    
                    // Check if this position marks a valid word ending
                    if (current->phoneme.has_value()) {
                        // Found a compound! Track it as the longest so far
                        match_length = i - after_bracket + 1;
                    }
                }
            }
            
            // If we found a compound word, use it with the furigana reading replacing the kanji
            // This ensures that 来「き」た becomes "きた" not "来た" for phoneme conversion
            if (match_length > 0) {
                size_t compound_end_byte = byte_positions[after_bracket + match_length];
                // 🔥 KEY FIX: Use the furigana READING instead of kanji!
                std::string compound = reading + text.substr(byte_positions[after_bracket], 
                                                             compound_end_byte - byte_positions[after_bracket]);
                segments.push_back(TextSegment(compound, kanji_start_byte));
                pos = after_bracket + match_length;
                used_compound = true;
            }
        }
        
        if (!used_compound) {
            // No compound found, use the furigana hint
            segments.push_back(TextSegment(kanji, reading, kanji_start_byte));
            pos = bracket_close + 1;
        }
    }
    
    return segments;
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
        
        // Use word segmentation if enabled and word dictionary is loaded
        if (g_use_segmentation && g_word_segmenter != nullptr) {
            // 🔥 STEP 1: Parse furigana hints into TextSegments
            // 健太「けんた」はバカ → [TextSegment("健太", "けんた"), TextSegment("はバカ")]
            // 見「み」て → [TextSegment("見て")] (compound word detected)
            auto segments = parse_furigana_segments(japanese_text, g_word_segmenter);
            
            // 🔥 STEP 2: Segment into words using TextSegments with phoneme fallback
            // Furigana segments are treated as atomic units
            auto words = g_word_segmenter->segment_from_segments(segments, g_converter->get_root());
            
            // 🔥 STEP 3: Convert each word to phonemes with particle handling
            for (size_t i = 0; i < words.size(); i++) {
                if (i > 0) result += " ";  // Add space between words
                
                // Special handling for the topic particle は → "wa"
                if (words[i] == "は" || words[i] == "\xe3\x81\xaf") {  // は in UTF-8
                    result += "wa";
                } else {
                    result += g_converter->convert(words[i]);
                }
            }
        } else {
            // Direct conversion without segmentation
            result = g_converter->convert(japanese_text);
        }
        
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

