#!/bin/bash
# Build script for Linux/macOS - Builds the native FFI library

set -e

echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo "║  Building Native Library for Dart FFI (Unix)           ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found! Please install CMake."
    exit 1
fi

mkdir -p build

echo "🔧 Configuring..."
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

echo ""
echo "🔨 Building..."
cmake --build build

echo ""
echo "📦 Copying output..."

if [[ "$OSTYPE" == "darwin"* ]]; then
    EXT="dylib"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    EXT="so"
else
    echo "❌ Unsupported platform: $OSTYPE"
    exit 1
fi

if [ -f "build/jpn_to_phoneme_ffi.$EXT" ]; then
    cp "build/jpn_to_phoneme_ffi.$EXT" ../
    echo "   ✓ Copied to dart_ffi/jpn_to_phoneme_ffi.$EXT"
fi

echo ""
echo "✨ Build complete!"
echo ""

