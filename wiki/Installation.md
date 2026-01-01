# Installation

This guide covers how to set up the toolkit for local development.

## Prerequisites

- C++17 compiler (MSVC, GCC, or Clang)
- CMake 3.15+
- Git
- Git Bash (for Windows)
- Emscripten SDK (for WebAssembly builds)

## Clone the Repository

```bash
git clone https://github.com/Calum-Kerr/HonoursProject.git
cd HonoursProject
```

## Initialize Submodules

The project uses vcpkg as a git submodule:

```bash
git submodule update --init --recursive
```

## Build the Backend

### Windows (PowerShell)

```powershell
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
cd ..
```

### Linux/macOS

```bash
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
cd ..
```

## Build WebAssembly (Optional)

To build the WASM version for offline mode:

### Install Emscripten

```bash
git clone https://github.com/emscripten-core/emsdk.git ../emsdk
cd ../emsdk
./emsdk install latest
./emsdk activate latest
cd ../HonoursProject
```

### Build WASM

```bash
source ../emsdk/emsdk_env.sh
bash build-wasm.sh
```

## Run Locally

**Important**: Run from the project root directory, not the build folder.

### Windows

```powershell
.\build\Debug\Toolkit.exe
```

### Linux/macOS

```bash
./build/Toolkit
```

Then open http://localhost:8080/app in your browser.

## Verify Installation

1. Open http://localhost:8080/app
2. Open browser developer console (F12)
3. You should see "WebAssembly loaded!" if WASM is working
4. You should see "Service Worker registered!" if PWA is working

