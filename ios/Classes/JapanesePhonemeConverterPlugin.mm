#import "JapanesePhonemeConverterPlugin.h"

// Forward declare the C++ FFI functions to ensure they're linked
// These are defined in jpn_to_phoneme_ffi.cpp
extern "C" {
    int jpn_phoneme_init(const char* json_file_path);
    int jpn_phoneme_init_from_memory(const uint8_t* trie_data, int data_size);
    int jpn_phoneme_convert(const char* japanese_text, uint8_t* output_buffer, int buffer_size, int64_t* processing_time_us);
    const char* jpn_phoneme_get_error(void);
    int jpn_phoneme_get_entry_count(void);
    const char* jpn_phoneme_version(void);
    int jpn_phoneme_init_word_dict(const char* word_file_path);
    void jpn_phoneme_set_use_segmentation(bool enabled);
    bool jpn_phoneme_get_use_segmentation(void);
    int jpn_phoneme_get_word_count(void);
    void jpn_phoneme_cleanup(void);
}

@implementation JapanesePhonemeConverterPlugin

+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  // FFI plugin - no method channel needed
  // The forward declarations above ensure the C++ symbols are linked
  // into the iOS app binary so they're available via DynamicLibrary.process()
  
  // Force linking by calling the helper method (prevents dead code stripping)
  // This ensures the linker includes all FFI symbols in the final binary
  [JapanesePhonemeConverterPlugin ensureFFISymbolsLinked];
}

// Add dummy method that references all FFI functions to prevent dead code stripping
// The iOS linker is aggressive about removing "unused" symbols
+ (void)ensureFFISymbolsLinked {
  // Reference each function to force the linker to include them
  // We don't actually call these, just reference their addresses
  void* symbols[] = {
    (void*)jpn_phoneme_init,
    (void*)jpn_phoneme_init_from_memory,
    (void*)jpn_phoneme_convert,
    (void*)jpn_phoneme_get_error,
    (void*)jpn_phoneme_get_entry_count,
    (void*)jpn_phoneme_version,
    (void*)jpn_phoneme_init_word_dict,
    (void*)jpn_phoneme_set_use_segmentation,
    (void*)jpn_phoneme_get_use_segmentation,
    (void*)jpn_phoneme_get_word_count,
    (void*)jpn_phoneme_cleanup
  };
  
  // Use the array to prevent compiler optimization
  volatile void* dummy = symbols[0];
  (void)dummy;
}

@end

