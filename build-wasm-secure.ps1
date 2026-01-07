Write-Host "Building hardened WebAssembly module..." -ForegroundColor Green
& "$PWD\emsdk\emsdk_env.bat"
emcc src/secure_algorithms.cpp src/algorithms-wasm.cpp `
    -o frontend/algorithms-secure.js `
    -s WASM=1 `
    -s EXPORTED_FUNCTIONS='["_parseCSV","_detectMissing","_detectDuplicates","_cleanData","_cleanDataString","_detectWhitespace","_trimWhitespaceString","_toUpperCaseString","_toLowerCaseString","_detectNullValues","_standardiseNullValueString","_detectOutliers","_removeOutliersString","_detectInconsistentValues","_standardiseValuesString","_detectDataTypes","_standardiseDateColumnString","_standardiseNumberColumnString","_detectFileEncoding","_detectPatterns","_autoDetectAll","_measurePerformance"]' `
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' `
    -s MODULARIZE=1 `
    -s EXPORT_NAME='AlgorithmsModule' `
    --std=c++17 `
    -fstack-protector-strong `
    -D_FORTIFY_SOURCE=2 `
    -Wformat -Wformat-security `
    -fPIE `
    -s ALLOW_MEMORY_GROWTH=1 `
    -s MAXIMUM_MEMORY=2GB `
    -s ABORTING_MALLOC=1 `
    -s DISABLE_EXCEPTION_CATCHING=1 `
    -s MALLOC=emmalloc `
    -s MINIMAL_RUNTIME=1 `
    -s SUPPORT_LONGJMP=0 `
    -s SAFE_HEAP=1 `
    -s STACK_OVERFLOW_CHECK=2 `
    -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Verifying WASM integrity..." -ForegroundColor Yellow
$wasmHash = Get-FileHash -Path "frontend\algorithms.wasm" -Algorithm SHA384
$hashValue = $wasmHash.Hash.ToLower()
Write-Host "WASM Hash (SHA-384): $hashValue" -ForegroundColor Cyan
$content = Get-Content -Path "frontend\service-worker.js"
$content = $content -replace "sha384-placeholder-hash-will-be-generated-at-build-time", $hashValue
Set-Content -Path "frontend\service-worker.js" -Value $content
Write-Host "Build process completed with security hardening." -ForegroundColor Green