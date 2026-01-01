# Troubleshooting

Common issues and how to fix them.

## Browser Issues

### Page is blank or not loading

**Cause:** Running the executable from the wrong directory.

**Solution:** Run the server from the project root, not the build folder:
```powershell
cd C:\Users\calum\OneDrive\Desktop\data-cleaning-toolkit-8
.\build\Debug\Toolkit.exe
```

### WebAssembly not loading

**Symptoms:** Results show `"mode": "api"` instead of `"mode": "wasm"`

**Possible causes:**
1. Browser does not support WebAssembly
2. WASM files not cached yet
3. Service worker not registered

**Solutions:**
1. Use a modern browser (Chrome, Firefox, Safari, Edge)
2. Hard refresh the page (Ctrl + Shift + R)
3. Check console for errors (F12)

### Service Worker errors

**Symptom:** Console shows "Failed to execute 'put' on 'Cache'"

**Cause:** Service worker trying to cache non-HTTP resources (like chrome-extension://)

**Solution:** This error can be ignored - it does not affect functionality.

### Install button not appearing

**Cause:** Browser blocks install prompt after previous dismissal.

**Solution:**
1. Open browser settings
2. Go to Site Settings for the toolkit URL
3. Reset permissions
4. Or manually install via browser menu

## Build Issues

### CMake cannot find vcpkg

**Cause:** Submodules not initialized.

**Solution:**
```bash
git submodule update --init --recursive
```

### Emscripten not found

**Cause:** Emscripten environment not activated.

**Solution:**
```bash
source ../emsdk/emsdk_env.sh
```

### WASM functions not exported

**Symptom:** "Cannot call unknown function" error in console

**Cause:** C++ functions missing EMSCRIPTEN_KEEPALIVE or extern "C"

**Solution:** Ensure functions in algorithms-wasm.cpp have:
```cpp
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    int functionName(const char* input) {
        // ...
    }
}
```

Then rebuild WASM:
```bash
bash build-wasm.sh
```

## Data Issues

### CSV not parsing correctly

**Possible causes:**
1. Non-standard delimiters (semicolons, tabs)
2. Quoted fields with commas inside
3. Different line endings (Windows vs Unix)

**Current limitations:**
- Only comma delimiter supported
- Quoted fields not fully supported
- Use standard CSV format

### Download returns original data

**Cause:** Cleaning function not updating the cleaned data variable.

**Solution:** Ensure the cleaning function stores result in `cleanedCSV` variable before download.

## Still having issues?

1. Check browser console (F12) for error messages
2. Try hard refresh (Ctrl + Shift + R)
3. Try a different browser
4. Clear site data and reload
5. Open an issue on GitHub
6. Email: calumxkerr@gmail.com

