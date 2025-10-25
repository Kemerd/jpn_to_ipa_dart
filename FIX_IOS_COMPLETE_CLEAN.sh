#!/bin/bash
# Complete iOS clean and rebuild script for japanese_phoneme_converter
# Run this from YOUR Flutter app directory, NOT from dart_ffi!

echo "🧹 Starting complete iOS clean..."

# Check if we're in a Flutter project
if [ ! -f "pubspec.yaml" ]; then
    echo "❌ Error: Not in a Flutter project directory. Run this from your app's root!"
    exit 1
fi

# Check if ios directory exists
if [ ! -d "ios" ]; then
    echo "❌ Error: No ios directory found!"
    exit 1
fi

echo "📍 Working in: $(pwd)"

# Step 1: Clean Flutter
echo "🔵 Step 1: Cleaning Flutter..."
flutter clean

# Step 2: Remove ALL iOS build artifacts
echo "🔵 Step 2: Removing iOS artifacts..."
cd ios
rm -rf Pods/
rm -rf .symlinks/
rm -f Podfile.lock
rm -rf ~/Library/Developer/Xcode/DerivedData/*
rm -rf build/
rm -rf Runner.xcworkspace/xcuserdata/
rm -rf Runner.xcworkspace/xcshareddata/

# Step 3: Clear CocoaPods caches
echo "🔵 Step 3: Clearing CocoaPods caches..."
pod cache clean --all
# Also clear the specific pod
pod cache clean 'japanese_phoneme_converter' --all 2>/dev/null || true

# Clear ALL CocoaPods caches (nuclear option)
rm -rf ~/Library/Caches/CocoaPods/Pods/Release/japanese_phoneme_converter* 2>/dev/null || true
rm -rf ~/Library/Caches/CocoaPods/Pods/External/japanese_phoneme_converter* 2>/dev/null || true
rm -rf ~/Library/Caches/CocoaPods/Pods/Specs/Release/japanese_phoneme_converter* 2>/dev/null || true

# Step 4: Return to app root
cd ..

# Step 5: Get Flutter dependencies
echo "🔵 Step 5: Getting Flutter dependencies..."
flutter pub get

# Step 6: Install pods fresh
echo "🔵 Step 6: Installing CocoaPods fresh..."
cd ios
pod install --repo-update

# Step 7: Verify the C++ file is in the project
echo "🔵 Step 7: Checking if jpn_to_phoneme_ffi.cpp is in Pods..."
if [ -f "Pods/japanese_phoneme_converter/native/jpn_to_phoneme_ffi.cpp" ]; then
    echo "✅ C++ file found in Pods!"
else
    echo "⚠️  Warning: C++ file not found in expected location"
    echo "   Checking other locations..."
    find Pods -name "jpn_to_phoneme_ffi.cpp" -type f 2>/dev/null
fi

# Step 8: Return to app root
cd ..

echo ""
echo "✅ Clean complete! Now try building:"
echo ""
echo "   flutter run -d iPhone"
echo ""
echo "If it still fails, run with verbose output:"
echo "   flutter run -d iPhone -v 2>&1 | grep -E '(jpn_to_phoneme|japanese_phoneme)'"
echo ""
echo "To check if symbols are in the binary after build:"
echo "   nm -gU build/ios/Debug-iphonesimulator/Runner.app/Runner | grep jpn_phoneme"
