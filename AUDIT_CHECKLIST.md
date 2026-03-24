# Data Cleaning Toolkit - Audit Checklist

## Project Overview
- **Type**: C++ backend + WebAssembly + JavaScript frontend
- **Purpose**: Privacy-first CSV data cleaning toolkit
- **Deployment**: Live at calumkerr.com/app
- **Architecture**: Crow REST API + WASM offline mode + Service Worker PWA

## Codebase Statistics
- **Total lines**: ~4,263 (C++ only)
- **Main files**: src/main.cpp (2,777 lines), src/algorithms-wasm.cpp (1,373 lines), src/algorithms.cpp (113 lines)
- **Frontend**: index.html, service-worker.js, manifest.json
- **Build**: CMake + vcpkg + Emscripten (for WASM)

## Existing Features - VERIFIED

### CSV Parsing
- ✅ Basic CSV parsing with quote handling
- ✅ Line-by-line parsing
- ✅ Handles quoted fields with commas

### Detection Features
- ✅ Detect missing values (empty cells)
- ✅ Detect duplicate rows
- ✅ Detect whitespace issues (leading/trailing)
- ✅ Detect null value representations (N/A, null, -, ?, ~, etc.)
- ✅ Detect outliers (IQR method)
- ✅ Detect inconsistent values (Levenshtein distance)
- ✅ Detect patterns (email, phone, postal codes)
- ✅ Auto-detect all issues in one call

### Cleaning Features
- ✅ Remove duplicate rows
- ✅ Trim whitespace
- ✅ Standardize case (uppercase/lowercase)
- ✅ Standardize null values
- ✅ Remove outliers
- ✅ Universal text cleaning (fuzzy matching, punctuation normalization)

### Advanced Features
- ✅ Fairness analysis (demographic parity, disparate impact)
- ✅ PDF report generation (GDPR-aligned)
- ✅ Audit logging (operation tracking)
- ✅ Data profiling (column statistics)
- ✅ Quality metrics calculation

### Backend (Crow)
- ✅ 20+ API endpoints
- ✅ Security headers (CSP, X-Frame-Options, etc.)
- ✅ Frontend asset serving
- ✅ SEO support (robots.txt, sitemap.xml, crawler detection)
- ✅ Favicon and manifest serving

### Frontend
- ✅ Single-page app (index.html)
- ✅ Drag-and-drop file upload
- ✅ Real-time detection/cleaning
- ✅ Data table display with pagination
- ✅ Before/after comparison
- ✅ CSV download
- ✅ Responsive design

### WebAssembly
- ✅ Compiled from C++ (algorithms-wasm.cpp)
- ✅ 30+ exported functions
- ✅ Offline processing
- ✅ Performance metrics

### Service Worker & PWA
- ✅ Offline caching
- ✅ WASM integrity verification
- ✅ Cache versioning
- ✅ Manifest.json for PWA

## Known Issues & Weaknesses

### Code Quality Issues
- ❌ main.cpp is 2,777 lines (violates 60-line rule)
- ❌ Many functions exceed 10-line limit
- ❌ Deep nesting (>3 levels) in several places
- ❌ Duplicated logic across main.cpp and algorithms-wasm.cpp
- ❌ Magic numbers without named constants
- ❌ Inconsistent error handling

### Architecture Issues
- ❌ Monolithic main.cpp contains all API routes
- ❌ No separation of concerns
- ❌ Utility functions scattered throughout
- ❌ CSV parsing logic duplicated in 3 places
- ❌ Text cleaning logic duplicated

### Performance Issues
- ⚠️ Fuzzy matching O(n²) complexity
- ⚠️ Large dataset handling untested
- ⚠️ Memory usage not optimized

### Testing
- ❌ No unit tests
- ❌ No integration tests
- ❌ Manual testing only

## Dead Code & Unused Features
- ⚠️ Some helper functions may be unused
- ⚠️ Secure algorithms header (secure_algorithms.h) not integrated

## Rebuild Priority
1. CSV parser (foundation)
2. Core utilities (whitespace, case, null handling)
3. Detection algorithms
4. Cleaning algorithms
5. Audit & metrics
6. PDF reporting
7. Backend API structure
8. Frontend integration
9. WASM compilation
10. Service worker & PWA

## Acceptance Criteria for Rebuild
- All features preserved and working
- Code follows 10 rules (60 lines/file, 10 lines/function, etc.)
- Bugs fixed (if any identified)
- Clean git history with small commits
- Tests pass
- Deployment successful

