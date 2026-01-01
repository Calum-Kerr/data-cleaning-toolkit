# Contributing Guidelines

Thank you for your interest in contributing to this project. While this is primarily a solo honours dissertation project for Edinburgh Napier University, i welcome feedback, bug reports, and suggestions.

## How to Contribute

### Reporting Bugs

If you find a bug, please open an issue on GitHub with:

- A clear and descriptive title
- Steps to reproduce the problem
- Expected behaviour vs actual behaviour
- Your browser and operating system
- Screenshots if applicable

### Suggesting Features

If you have an idea for a new feature, please open an issue with:

- A clear description of the feature
- Why it would be useful
- Any examples of similar features in other tools

### Code Contributions

If you want to contribute code:

1. Fork the repository
2. Create a new branch for your feature (`git checkout -b feature/your-feature-name`)
3. Make your changes
4. Test your changes locally
5. Commit with clear, descriptive messages
6. Push to your fork
7. Open a pull request

### Pull Request Guidelines

- Keep changes focused and minimal
- Follow existing code style
- Test both WASM and API modes
- Update documentation if needed
- Reference any related issues

## Development Setup

### Prerequisites

- C++17 compiler
- CMake
- vcpkg (included as submodule)
- Emscripten (for WebAssembly builds)
- Git Bash (for Windows)

### Building the Backend

```bash
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
