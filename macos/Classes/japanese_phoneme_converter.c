// Relative import to be able to reuse the C++ sources.
// See the comment in ../japanese_phoneme_converter.podspec for more information.
#include "../../src/jpn_to_phoneme_ffi.cpp"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CRITICAL: Symbol Stub for Static Linking
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// When building as a static framework, symbols aren't automatically added to
// RTLD_DEFAULT (global symbol table) unless they're referenced. This stub
// creates a reference to all FFI functions, forcing them into the global table
// so DynamicLibrary.process() can find them via dlsym().
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

#ifdef __cplusplus
extern "C" {
#endif

// This function is never called - it just forces the linker to include all symbols
__attribute__((used))
__attribute__((visibility("default")))
static void* japanese_phoneme_converter_symbol_stub(void) {
    // Create array of function pointers to all FFI exports
    static void* symbols[] = {
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
        (void*)jpn_phoneme_cleanup,
        nullptr
    };
    return symbols[0];
}

#ifdef __cplusplus
}
#endif