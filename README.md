# Data Cleaning Toolkit

This will be a transparent toolkit for data cleaning using C++ with a web based PWA interface.

## How to run if freshly cloned!

git submodule update --init --recursive

Set-Location vcpkg

.\bootstrap-vcpkg.bat

Set-Location ..

.\vcpkg\vcpkg install asio:x64-windows boost-algorithm:x64-windows boost-functional:x64-windows boost-optional:x64-windows boost-lexical-cast:x64-windows boost-array:x64-windows

mkdir build

Set-Location build

cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"

cmake --build .

.\Debug\Toolkit.exe

##run if build already exists

.\build\Debug\Toolkit.exe