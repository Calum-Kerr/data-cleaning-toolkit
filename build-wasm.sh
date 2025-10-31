#!/bin/bash
EMCC="emsdk/upstream/emscripten/emcc"
$EMCC src/algorithms-wasm.cpp\
    -o frontend/algorithms.js\
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_parseCSV","_detectMissing","_detectDuplicates","_cleanData"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='AlgorithmsModule' \
    --std=c++17