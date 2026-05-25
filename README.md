# Data Cleaning Toolkit

A transparent, privacy-first data cleaning toolkit built in C++ with an offline-capable web frontend.

**Try the live toolkit: [tidy.calumkerr.com/app](https://tidy.calumkerr.com/app)**

No sign-up. Free. Privacy-first — data is processed in memory, never stored.

[![Live Demo](https://img.shields.io/badge/demo-live-brightgreen)](https://tidy.calumkerr.com/app)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## About

This toolkit helps you clean CSV data by detecting and fixing common data quality issues. It is developed as an honours dissertation project for Edinburgh Napier University.

**Key Features:**
- **Transparent**: All algorithms implemented from scratch and available for inspection
- **Privacy-First**: data is processed in memory only, never stored or shared. Client-side WebAssembly processing is in active development.
- **No Coding Required**: Simple web interface for uploading and cleaning CSV files
- **Reproducible**: Same input always produces same output
- **Offline-Capable Shell**: the app interface loads and stays usable offline; cleaning operations require a server connection.

## Features

| Feature | Detection | Cleaning |
|---------|-----------|----------|
| Missing Values | yes | - |
| Duplicate Rows | yes | yes |
| Whitespace Issues | yes | yes |
| Case Inconsistency | yes | yes |
| Null Representations | yes | yes |

## Quick Start

### Option 1: Use the Live Demo

1. Go to https://tidy.calumkerr.com/app
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
git clone https://github.com/Calum-Kerr/data-cleaning-toolkit.git
cd data-cleaning-toolkit

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

## Project Structure

```
data-cleaning-toolkit/
├── backend/
│   ├── src/
│   │   ├── core/                # cleaning algorithms (deduplication, clustering, etc.)
│   │   ├── routes/              # HTTP route handlers
│   │   ├── parsers/             # CSV parser and serialiser
│   │   ├── text/                # text normalisation and domain cleaners
│   │   ├── platform/            # logging, rate limiting, audit, analytics
│   │   └── main.cpp
│   ├── vendor/
│   │   └── crow_all.h           # Crow HTTP framework (header-only)
│   └── CMakeLists.txt
├── frontend/
│   ├── index.html               # main app
│   ├── home.html                # landing page
│   ├── features.html
│   ├── honours-project.html
│   ├── privacy.html
│   ├── offline.html
│   ├── css/
│   │   └── tokens.css
│   ├── js/
│   │   ├── app/                 # app-specific modules
│   │   ├── vendor/              # jsPDF (vendored, not from CDN)
│   │   ├── service-worker.js
│   │   └── register-sw.js
│   ├── samples/                 # example datasets
│   ├── robots.txt
│   ├── sitemap.xml
│   └── manifest.json
├── vcpkg/                       # C++ dependency manager (not committed)
├── .github/
│   └── workflows/
│       └── deploy.yml
└── README.md
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
- `POST /api/standardise-case` - standardise case
- `POST /api/standardise-null-values` - standardise nulls

## Documentation

The research context, methodology, and design decisions behind this toolkit are documented at [tidy.calumkerr.com/honours-project](https://tidy.calumkerr.com/honours-project).

The source code is self-documenting where possible — see comments in `backend/src/` for implementation notes.

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
