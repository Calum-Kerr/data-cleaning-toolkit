#!/bin/bash

set -e

echo "=========================================="
echo "Building Data Cleaning Toolkit"
echo "=========================================="

# Create build directory
mkdir -p build
cd build

# Compile all source files
echo "Compiling source files..."
clang++ -std=c++17 -fPIE -fstack-protector-strong \
  -I../include \
  -I/usr/include \
  ../src/main.cpp \
  ../src/csv_parser.cpp \
  ../src/text_utils.cpp \
  ../src/text_utils2.cpp \
  ../src/detectors.cpp \
  ../src/detectors2.cpp \
  ../src/cleaners.cpp \
  ../src/cleaners2.cpp \
  ../src/cleaners3.cpp \
  ../src/audit.cpp \
  ../src/pdf_report.cpp \
  ../src/routes.cpp \
  ../src/routes2.cpp \
  ../src/routes3.cpp \
  -pthread \
  -o Toolkit \
  2>&1

if [ $? -eq 0 ]; then
  echo "✅ Build successful!"
  echo "Binary: ./build/Toolkit"
  ls -lh Toolkit
else
  echo "❌ Build failed!"
  exit 1
fi

