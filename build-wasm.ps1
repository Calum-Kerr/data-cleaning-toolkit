# Activate emsdk environment
& "emsdk/emsdk_env.ps1"

# Run emcc to compile WASM
emcc src/algorithms-wasm.cpp `
    -o frontend/algorithms.js `
    -s WASM=1 `
    -s EXPORTED_FUNCTIONS='["_parseCSV","_detectMissing","_detectDuplicates","_cleanData","_cleanDataString","_detectWhitespace","_trimWhitespaceString","_toUpperCaseString","_toLowerCaseString","_detectNullValues","_standardiseNullValueString","_detectOutliers","_removeOutliersString","_detectInconsistentValues","_standardiseValuesString","_detectDataTypes","_standardiseDateColumnString","_standardiseNumberColumnString","_detectFileEncoding"]' `
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' `
    -s MODULARIZE=1 `
    -s EXPORT_NAME='AlgorithmsModule' `
    --std=c++17

Write-Host "WASM build complete!"

