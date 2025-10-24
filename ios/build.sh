#!/bin/bash

# Japanese Phoneme Converter - iOS Build Script
# This script builds the native C++ library for iOS platforms
# including both device and simulator architectures

set -e  # Exit on error

echo "🔨 Building Japanese Phoneme Converter for iOS..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/../.."
FFI_SOURCE="$SCRIPT_DIR/../native/jpn_to_phoneme_ffi.cpp"
BUILD_DIR="$SCRIPT_DIR/build"
FRAMEWORK_NAME="JapanesePhonemeConverter"

# Check if source file exists
if [ ! -f "$FFI_SOURCE" ]; then
    echo -e "${RED}❌ Error: FFI source file not found at $FFI_SOURCE${NC}"
    exit 1
fi

# Create build directory
echo "📁 Creating build directory..."
mkdir -p "$BUILD_DIR"

# Function to build for a specific SDK and architecture
build_arch() {
    local SDK=$1
    local ARCH=$2
    local PLATFORM=$3
    local OUTPUT_DIR="$BUILD_DIR/$PLATFORM-$ARCH"
    
    echo -e "${YELLOW}🏗️  Building for $PLATFORM ($ARCH)...${NC}"
    
    mkdir -p "$OUTPUT_DIR"
    
    # Compiler flags
    CFLAGS="-arch $ARCH -isysroot $(xcrun -sdk $SDK --show-sdk-path) -std=c++17"
    CFLAGS="$CFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
    CFLAGS="$CFLAGS -fembed-bitcode"
    
    # Optimization flags for release build
    CFLAGS="$CFLAGS -O3 -ffast-math -funroll-loops"
    
    # Define minimum iOS version
    if [ "$PLATFORM" == "iphoneos" ]; then
        CFLAGS="$CFLAGS -miphoneos-version-min=11.0"
    else
        CFLAGS="$CFLAGS -mios-simulator-version-min=11.0"
    fi
    
    # Build the object file
    xcrun -sdk $SDK clang++ $CFLAGS -c "$FFI_SOURCE" -o "$OUTPUT_DIR/jpn_to_phoneme_ffi.o"
    
    # Create static library
    xcrun -sdk $SDK ar rcs "$OUTPUT_DIR/lib$FRAMEWORK_NAME.a" "$OUTPUT_DIR/jpn_to_phoneme_ffi.o"
    
    echo -e "${GREEN}✅ Built $PLATFORM ($ARCH)${NC}"
}

# Build for iOS devices (arm64)
build_arch iphoneos arm64 iphoneos

# Build for iOS Simulator (x86_64 for Intel Macs)
build_arch iphonesimulator x86_64 iphonesimulator

# Build for iOS Simulator (arm64 for M1 Macs)
build_arch iphonesimulator arm64 iphonesimulator-arm64

# Create universal binary for simulator
echo -e "${YELLOW}🔗 Creating universal simulator library...${NC}"
mkdir -p "$BUILD_DIR/iphonesimulator-universal"
lipo -create \
    "$BUILD_DIR/iphonesimulator-x86_64/lib$FRAMEWORK_NAME.a" \
    "$BUILD_DIR/iphonesimulator-arm64/lib$FRAMEWORK_NAME.a" \
    -output "$BUILD_DIR/iphonesimulator-universal/lib$FRAMEWORK_NAME.a"

# Create XCFramework
echo -e "${YELLOW}📦 Creating XCFramework...${NC}"
XCFRAMEWORK_PATH="$BUILD_DIR/$FRAMEWORK_NAME.xcframework"
rm -rf "$XCFRAMEWORK_PATH"

xcodebuild -create-xcframework \
    -library "$BUILD_DIR/iphoneos-arm64/lib$FRAMEWORK_NAME.a" \
    -library "$BUILD_DIR/iphonesimulator-universal/lib$FRAMEWORK_NAME.a" \
    -output "$XCFRAMEWORK_PATH"

# Copy to plugin directory
echo -e "${YELLOW}📋 Copying framework to plugin directory...${NC}"
PLUGIN_FRAMEWORKS_DIR="$SCRIPT_DIR/Frameworks"
mkdir -p "$PLUGIN_FRAMEWORKS_DIR"
rm -rf "$PLUGIN_FRAMEWORKS_DIR/$FRAMEWORK_NAME.xcframework"
cp -R "$XCFRAMEWORK_PATH" "$PLUGIN_FRAMEWORKS_DIR/"

# Create module map for Swift/Objective-C interop
echo -e "${YELLOW}📝 Creating module map...${NC}"
MODULE_DIR="$PLUGIN_FRAMEWORKS_DIR/$FRAMEWORK_NAME.xcframework/ios-arm64/Headers"
mkdir -p "$MODULE_DIR"

cat > "$MODULE_DIR/module.modulemap" << EOF
module JapanesePhonemeConverter {
    header "jpn_to_phoneme_ffi.h"
    export *
}
EOF

# Create header file with FFI function declarations
cat > "$MODULE_DIR/jpn_to_phoneme_ffi.h" << 'EOF'
#ifndef JPN_TO_PHONEME_FFI_H
#define JPN_TO_PHONEME_FFI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Initialization functions
int jpn_phoneme_init(const char* json_file_path);
int jpn_phoneme_init_from_memory(const uint8_t* trie_data, int data_size);

// Conversion function
int jpn_phoneme_convert(
    const char* japanese_text,
    uint8_t* output_buffer,
    int buffer_size,
    int64_t* processing_time_us
);

// Error handling
const char* jpn_phoneme_get_error(void);

// Information functions
int jpn_phoneme_get_entry_count(void);
const char* jpn_phoneme_version(void);

// Word segmentation functions
int jpn_phoneme_init_word_dict(const char* word_file_path);
void jpn_phoneme_set_use_segmentation(bool enabled);
bool jpn_phoneme_get_use_segmentation(void);
int jpn_phoneme_get_word_count(void);

// Cleanup
void jpn_phoneme_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // JPN_TO_PHONEME_FFI_H
EOF

# Update podspec to include the framework
echo -e "${YELLOW}📝 Updating podspec...${NC}"
PODSPEC_PATH="$SCRIPT_DIR/japanese_phoneme_converter.podspec"

# Create a temporary podspec update script
cat > "$BUILD_DIR/update_podspec.rb" << 'EOF'
podspec_path = ARGV[0]
content = File.read(podspec_path)

# Check if vendored_frameworks is already in the podspec
unless content.include?('vendored_frameworks')
  # Add vendored_frameworks after the resources line
  updated_content = content.gsub(
    /s\.resources = \['..\/assets\/\*'\]/,
    "s.resources = ['../assets/*']\n  \n  # Include pre-built XCFramework\n  s.vendored_frameworks = 'Frameworks/JapanesePhonemeConverter.xcframework'"
  )
  
  # Also update source_files to not include the C++ file since we're using pre-built framework
  updated_content.gsub!(
    /s\.source_files = \['Classes\/\*\*\/\*', '..\/native\/jpn_to_phoneme_ffi\.cpp'\]/,
    "s.source_files = 'Classes/**/*'"
  )
  
  # Remove preserve_paths for the C++ file
  updated_content.gsub!(/s\.preserve_paths = '..\/native\/jpn_to_phoneme_ffi\.cpp'\n/, '')
  
  File.write(podspec_path, updated_content)
  puts "✅ Updated podspec"
else
  puts "ℹ️  Podspec already contains vendored_frameworks"
end
EOF

ruby "$BUILD_DIR/update_podspec.rb" "$PODSPEC_PATH"

# Summary
echo -e "${GREEN}"
echo "================================="
echo "✅ iOS Build Complete!"
echo "================================="
echo -e "${NC}"
echo "📦 XCFramework location: $PLUGIN_FRAMEWORKS_DIR/$FRAMEWORK_NAME.xcframework"
echo ""
echo "🚀 Next steps:"
echo "1. Run 'pod install' in your iOS project"
echo "2. The framework will be automatically linked"
echo "3. Test with: flutter run -d <ios-device-id>"
echo ""
echo "💡 Tips:"
echo "- For debugging, add -DDEBUG to CFLAGS"
echo "- Check Console.app for runtime logs"
echo "- Use 'flutter build ios --release' for production builds"

# Clean up temporary files
rm -f "$BUILD_DIR/update_podspec.rb"

echo -e "${GREEN}✨ Done!${NC}"
