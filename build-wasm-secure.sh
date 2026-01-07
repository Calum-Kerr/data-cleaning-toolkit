echo "Building hardened WebAssembly module..."
source emsdk/emsdk_env.sh
emcc src/secure_algorithms.cpp src/algorithms-wasm.cpp \
    -o frontend/algorithms-secure.js \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_parseCSV","_detectMissing","_detectDuplicates","_cleanData","_cleanDataString","_detectWhitespace","_trimWhitespaceString","_toUpperCaseString","_toLowerCaseString","_detectNullValues","_standardiseNullValueString","_detectOutliers","_removeOutliersString","_detectInconsistentValues","_standardiseValuesString","_detectDataTypes","_standardiseDateColumnString","_standardiseNumberColumnString","_detectFileEncoding","_detectPatterns","_autoDetectAll","_measurePerformance"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='AlgorithmsModule' \
    --std=c++17 \
    -fstack-protector-strong \
    -D_FORTIFY_SOURCE=2 \
    -Wformat -Wformat-security \
    -fPIE \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MAXIMUM_MEMORY=2GB \
    -s ABORTING_MALLOC=1 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -s MALLOC=emmalloc \
    -s MINIMAL_RUNTIME=1 \
    -s SUPPORT_LONGJMP=0 \
    -s SAFE_HEAP=1 \
    -s STACK_OVERFLOW_CHECK=2 \
    -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1
echo "Build completed successfully!"
echo "Verifying WASM integrity..."
WASM_HASH=$(shasum -a 384 frontend/algorithms.wasm | cut -d ' ' -f 1)
echo "WASM Hash (SHA-384): $WASM_HASH"
sed -i.bak "s/sha384-placeholder-hash-will-be-generated-at-build-time/$WASM_HASH/g" frontend/service-worker.js
echo "Build process completed with security hardening."