# Frequently Asked Questions

## General

### What is this toolkit for?

This toolkit helps you clean CSV data by detecting and fixing common data quality issues like missing values, duplicates, whitespace problems, and inconsistent null representations.

### Is my data safe?

Yes. When using offline mode (WASM), your data never leaves your device. It is processed entirely in your browser using WebAssembly.

### Do I need to install anything?

No. The toolkit runs in your web browser. Just visit the demo URL and start using it.

### What browsers are supported?

Any modern browser that supports WebAssembly:
- Chrome 57+
- Firefox 52+
- Safari 11+
- Edge 16+

## Usage

### How do I know if I am using offline or online mode?

The result of each operation shows a "mode" field:
- `"mode": "wasm"` = offline mode (data stays on your device)
- `"mode": "api"` = online mode (data sent to server)

### Can I use this without an internet connection?

Yes, after visiting the demo once while online. The service worker caches all necessary files for offline use.

### What file formats are supported?

Currently only CSV (comma-separated values) files are supported.

### How large can my files be?

There is no hard limit, but very large files may be slow to process in the browser. For files over 100MB, consider using the API directly.

### Does this work on mobile?

Yes. The toolkit is a Progressive Web App (PWA) and can be installed on mobile devices.

## Technical

### Why C++ instead of JavaScript?

C++ was chosen because:
1. Dissertation requirement for implementing algorithms from scratch
2. Can be compiled to WebAssembly for browser use
3. Same codebase works for both offline and online modes
4. Demonstrates transparency (source code is inspectable)

### What is WebAssembly?

WebAssembly (WASM) is a binary instruction format that runs in web browsers at near-native speed. It allows the C++ algorithms to run directly in your browser.

### Can I use the API programmatically?

Yes. See the [API Reference](API-Reference) for endpoint documentation.

### Is the source code available?

Yes. All source code is available on GitHub and can be inspected to verify exactly how the algorithms work. This transparency is a key feature of the project.

## Troubleshooting

See the [Troubleshooting](Troubleshooting) page for common issues and solutions.

