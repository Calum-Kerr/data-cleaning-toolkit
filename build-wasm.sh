#!/bin/bash
export PATH="emsdk/upstream/emscripten:emsdk/upstream/bin:emsdk/node/22.16.0_64bit/bin:$PATH"
emsdk/upstream/emscripten/emcc src/algorithms-wasm.cpp\
    -o frontend/algorithms.js\
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_parseCSV","_detectMissing","_detectDuplicates","_cleanData","_cleanDataString","_detectWhitespace","_trimWhitespaceString","_toUpperCaseString","_toLowerCaseString","_detectNullValues","_standardiseNullValueString","_detectOutliers","_removeOutliersString","_detectInconsistentValues","_standardiseValuesString","_detectDataTypes","_standardiseDateColumnString","_standardiseNumberColumnString","_detectFileEncoding","_detectPatterns","_autoDetectAll","_measurePerformance","_profileColumn","_removeAllDuplicatesString","_removeAllDuplicatesStringStreaming","_quickCleanAllStreaming","_extractFirstTokenString","_fuzzyMatchAndMergeString","_extractNTokensString"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='AlgorithmsModule' \
    --std=c++17 \
    -fstack-protector-strong \
    -D_FORTIFY_SOURCE=2 \
    -Wformat -Wformat-security \
    -fPIE \
    -s INITIAL_MEMORY=268435456 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MAXIMUM_MEMORY=2GB \
    -s ABORTING_MALLOC=1