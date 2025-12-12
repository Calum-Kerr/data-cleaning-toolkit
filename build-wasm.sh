#!/bin/bash
EMCC="emsdk/upstream/emscripten/emcc"
$EMCC src/algorithms-wasm.cpp\
    -o frontend/algorithms.js\
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_parseCSV","_detectMissing","_detectDuplicates","_cleanData","_cleanDataString","_detectWhitespace","_trimWhitespaceString","_toUpperCaseString","_toLowerCaseString","_detectNullValues","_standardiseNullValueString"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='AlgorithmsModule' \
    --std=c++17