#!/bin/bash

# Test runner script for Data Cleaning Toolkit

echo "Data Cleaning Toolkit - Test Runner"
echo "===================================="

if [ -z "$1" ]; then
    echo ""
    echo "Usage:"
    echo "  ./test_runner.sh unit    - Run C++ unit tests"
    echo "  ./test_runner.sh e2e     - Run Playwright E2E tests"
    echo "  ./test_runner.sh all     - Run all tests"
    echo ""
    exit 1
fi

case $1 in
    "unit")
        echo ""
        echo "Running C++ Unit Tests..."
        echo "-------------------------"
        if [ ! -d "build-tests" ]; then
            echo "Creating build directory for tests..."
            mkdir -p build-tests
        fi
        cd build-tests
        cmake .. -DBUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"
        if [ $? -ne 0 ]; then
            echo "Error: CMake configuration failed"
            exit 1
        fi
        cmake --build .
        if [ $? -ne 0 ]; then
            echo "Error: Build failed"
            exit 1
        fi
        echo ""
        echo "Running unit tests..."
        ./runUnitTests
        cd ..
        ;;
    "e2e")
        echo ""
        echo "Running E2E Tests..."
        echo "--------------------"
        cd tests/e2e
        npm test
        cd ../..
        ;;
    "all")
        ./test_runner.sh unit
        ./test_runner.sh e2e
        ;;
    *)
        echo ""
        echo "Usage:"
        echo "  ./test_runner.sh unit    - Run C++ unit tests"
        echo "  ./test_runner.sh e2e     - Run Playwright E2E tests"
        echo "  ./test_runner.sh all     - Run all tests"
        echo ""
        ;;
esac