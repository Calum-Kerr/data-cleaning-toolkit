# Data Cleaning Toolkit

A transparent, privacy-first data cleaning toolkit built in C++ with WebAssembly for offline use.

[![Live Demo](https://img.shields.io/badge/demo-live-brightgreen)](https://www.calumkerr.com/app)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## About

This toolkit helps you clean CSV data by detecting and fixing common data quality issues. It is developed as an honours dissertation project for Edinburgh Napier University.

**Key Features:**
- **Transparent**: All algorithms implemented from scratch and available for inspection
- **Privacy-First**: Runs offline via WebAssembly - data never leaves your device
- **No Coding Required**: Simple web interface for uploading and cleaning CSV files
- **Reproducible**: Same input always produces same output
- **Hybrid Mode**: Works offline (WASM) or online (API) with automatic fallback

## Live Demo

Try it now: **https://www.calumkerr.com/app**

## Features

| Feature | Detection | Cleaning |
|---------|-----------|----------|
| Missing Values | ✅ | - |
| Duplicate Rows | ✅ | ✅ |
| Whitespace Issues | ✅ | ✅ |
| Case Inconsistency | ✅ | ✅ |
| Null Representations | ✅ | ✅ |

## Quick Start

### Option 1: Use the Live Demo

1. Go to https://www.calumkerr.com/app
2. Upload a CSV file
3. Click detection buttons to find issues
4. Click cleaning buttons to fix issues
5. Download cleaned CSV

### Option 2: Run Locally

See [Installation](#installation) below.

## Installation

### Prerequisites

Install these first (one-time setup):

```bash
winget install Kitware.CMake
winget install Microsoft.VisualStudio.2022.BuildTools
```

### Fresh Clone Setup

```powershell
git clone https://github.com/Calum-Kerr/HonoursProject.git
cd HonoursProject

git submodule update --init --recursive

cd vcpkg
.\bootstrap-vcpkg.bat
cd ..

.\vcpkg\vcpkg install asio:x64-windows boost-algorithm:x64-windows boost-functional:x64-windows boost-optional:x64-windows boost-lexical-cast:x64-windows boost-array:x64-windows

mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build .
```

### Run the Server

```powershell
cd ..
.\build\Debug\Toolkit.exe
```

Then open http://localhost:8080/app

### Quick Rebuild (if build exists)

PowerShell:
```powershell
.\build\Debug\Toolkit.exe
```

Git Bash:
```bash
cd build && cmake --build . && cd ..
./build/Debug/Toolkit.exe
```

## Building WebAssembly (Optional)

For offline mode, build the WASM version:

```bash
# Install Emscripten first
git clone https://github.com/emscripten-core/emsdk.git ../emsdk
cd ../emsdk && ./emsdk install latest && ./emsdk activate latest && cd ../HonoursProject

# Build WASM
source ../emsdk/emsdk_env.sh
bash build-wasm.sh
```

## Project Structure

```
├── src/
│   ├── main.cpp              # Backend server
│   ├── algorithms.cpp        # C++ algorithms
│   └── algorithms-wasm.cpp   # WASM-compatible functions
├── frontend/
│   ├── index.html            # Web interface
│   ├── algorithms.js         # WASM loader
│   └── algorithms.wasm       # Compiled WebAssembly
├── .github/                  # Issue templates, PR template
└── build/                    # Build output
```

## API Endpoints

- `GET /` - Health check
- `GET /app` - Web interface
- `POST /api/parse` - Parse CSV
- `POST /api/detect-missing` - Find missing values
- `POST /api/detect-duplicates` - Find duplicates
- `POST /api/detect-whitespace` - Find whitespace issues
- `POST /api/detect-null-values` - Find null representations
- `POST /api/clean` - Remove duplicates
- `POST /api/trim-whitespace` - Trim whitespace
- `POST /api/standardize-case` - Standardize case
- `POST /api/standardize-null-values` - Standardize nulls

## Documentation

- [Getting Started](wiki/Getting-Started.md)
- [Usage Guide](wiki/Usage-Guide.md)
- [API Reference](wiki/API-Reference.md)
- [Algorithms](wiki/Algorithms.md)
- [FAQ](wiki/FAQ.md)
- [Troubleshooting](wiki/Troubleshooting.md)

## Contributing

Contributions are welcome! Please read the [Contributing Guidelines](CONTRIBUTING.md) first.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file.

## Author

**Calum Kerr** - Honours Project 2025

- Email: calumxkerr@gmail.com
- GitHub: [@Calum-Kerr](https://github.com/Calum-Kerr)

## Acknowledgments

This project is an honours dissertation for Edinburgh Napier University, supervised by Taoxin Peng.

