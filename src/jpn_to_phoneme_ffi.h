/**
 * @file jpn_to_phoneme_ffi.h
 * @brief C API for Japanese Phoneme Converter FFI
 * 
 * This header provides C-compatible function declarations for Dart FFI bindings.
 * 
 * @author Japanese Phoneme Converter Contributors
 * @version 2.0.0
 */

#ifndef JPN_TO_PHONEME_FFI_H
#define JPN_TO_PHONEME_FFI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// INITIALIZATION FUNCTIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * @brief Initialize the phoneme converter with a JSON dictionary file
 * 
 * @param json_file_path Path to the ja_phonemes.json file (UTF-8 encoded)
 * @return 1 on success, 0 on failure
 */
int jpn_phoneme_init(const char* json_file_path);

/**
 * @brief Initialize the phoneme converter from memory-mapped .trie data
 * 
 * @param trie_data Pointer to the binary .trie data in memory
 * @param data_size Size of the trie data in bytes
 * @return 1 on success, 0 on failure
 */
int jpn_phoneme_init_from_memory(const uint8_t* trie_data, int data_size);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CONVERSION FUNCTIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * @brief Convert Japanese text to IPA phonemes
 * 
 * @param japanese_text Input Japanese text (UTF-8 encoded, null-terminated)
 * @param output_buffer Buffer to store the resulting phonemes (UTF-8 encoded)
 * @param buffer_size Size of the output buffer in bytes
 * @param processing_time_us Pointer to store processing time in microseconds (can be NULL)
 * @return Number of bytes written to output_buffer, or -1 on error
 */
int jpn_phoneme_convert(
    const char* japanese_text,
    uint8_t* output_buffer,
    int buffer_size,
    int64_t* processing_time_us
);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ERROR HANDLING FUNCTIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * @brief Get the last error message
 * 
 * @return Error message string (never NULL, empty string if no error)
 */
const char* jpn_phoneme_get_error(void);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// INFORMATION FUNCTIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * @brief Get the number of entries in the phoneme dictionary
 * 
 * @return Number of entries, or -1 if not initialized
 */
int jpn_phoneme_get_entry_count(void);

/**
 * @brief Get the library version string
 * 
 * @return Version string (e.g., "2.0.0")
 */
const char* jpn_phoneme_version(void);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// WORD SEGMENTATION FUNCTIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * @brief Initialize the word dictionary for segmentation
 * 
 * @param word_file_path Path to the word list file (e.g., "ja_words.txt")
 * @return 1 on success, 0 on failure
 */
int jpn_phoneme_init_word_dict(const char* word_file_path);

/**
 * @brief Enable or disable word segmentation
 * 
 * @param enabled true to enable segmentation, false to disable
 */
void jpn_phoneme_set_use_segmentation(bool enabled);

/**
 * @brief Check if word segmentation is enabled
 * 
 * @return true if segmentation is enabled, false otherwise
 */
bool jpn_phoneme_get_use_segmentation(void);

/**
 * @brief Get the number of words in the word dictionary
 * 
 * @return Number of words loaded, or -1 if no dictionary loaded
 */
int jpn_phoneme_get_word_count(void);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CLEANUP FUNCTIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * @brief Clean up all resources used by the converter
 */
void jpn_phoneme_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // JPN_TO_PHONEME_FFI_H

