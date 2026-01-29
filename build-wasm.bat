@echo off
setlocal enabledelayedexpansion

REM Set up the environment
set EMSDK=%CD%\emsdk
set PATH=%EMSDK%\upstream\emscripten;%EMSDK%\upstream\bin;%EMSDK%\node\22.16.0_64bit\bin;%PATH%

REM Run emcc
call %EMSDK%\upstream\emscripten\emcc src/algorithms-wasm.cpp ^
    -o frontend/algorithms.js ^
    -s WASM=1 ^
    -s "EXPORTED_FUNCTIONS=[_testStandardize,_profileColumn,_removeEmptyRowsString,_removeAllDuplicatesString]" ^
    -s "EXPORTED_RUNTIME_METHODS=[ccall,cwrap]" ^
    -s MODULARIZE=1 ^
    -s EXPORT_NAME=AlgorithmsModule ^
    --std=c++17 ^
    -fstack-protector-strong ^
    -D_FORTIFY_SOURCE=2 ^
    -Wformat -Wformat-security ^
    -fPIE ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s MAXIMUM_MEMORY=2GB ^
    -s ABORTING_MALLOC=1

endlocal

