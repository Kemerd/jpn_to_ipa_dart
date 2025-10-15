@echo off
REM Build script for Windows - Builds the native FFI library

echo.
echo ╔════════════════════════════════════════════════════════╗
echo ║  Building Native Library for Dart FFI (Windows)        ║
echo ╚════════════════════════════════════════════════════════╝
echo.

cmake --version >nul 2>&1
if errorlevel 1 (
    echo ❌ CMake not found! Please install CMake.
    exit /b 1
)

if not exist build mkdir build

echo 🔧 Configuring...
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1

echo.
echo 🔨 Building...
cmake --build build --config Release
if errorlevel 1 exit /b 1

echo.
echo 📦 Copying output...
if exist build\Release\jpn_to_phoneme_ffi.dll (
    copy build\Release\jpn_to_phoneme_ffi.dll ..\ >nul
    echo    ✓ Copied to dart_ffi\jpn_to_phoneme_ffi.dll
)

echo.
echo ✨ Build complete!
echo.

