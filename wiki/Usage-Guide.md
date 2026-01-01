# Usage Guide

This guide explains how to use each feature of the Data Cleaning Toolkit.

## Uploading Data

1. Click the file input or drag and drop a CSV file
2. The toolkit accepts standard CSV format (comma-separated values)
3. The first row is treated as headers

## Detection Features

### Detect Missing Values

Finds empty cells in your data.

- Click "Detect Missing Values"
- Result shows count of empty cells found
- Mode indicates whether WASM or API was used

### Detect Duplicates

Finds rows that appear more than once.

- Click "Detect Duplicates"
- Result shows count of duplicate rows found
- First occurrence is not counted as duplicate

### Detect Whitespace

Finds cells with leading or trailing whitespace.

- Click "Detect Whitespace"
- Result shows count of cells with whitespace issues

### Detect Null Values

Finds various representations of null/missing values.

Recognized null representations:
- Empty string
- N/A, n/a, NA
- null, NULL
- None, NONE
- - (dash)
- ? (question mark)

## Cleaning Features

### Remove Duplicates

Removes duplicate rows from your data.

- Click "Clean Data" or "Remove Duplicates"
- Keeps the first occurrence of each unique row
- Result shows original, cleaned, and removed row counts

### Trim Whitespace

Removes leading and trailing whitespace from all cells.

- Click "Trim Whitespace"
- Result shows count of cells trimmed
- Download button appears to get cleaned data

### Standardize Case

Converts text to consistent case.

- Click "Uppercase" or "Lowercase"
- Applies to all text cells
- Result shows count of cells converted

### Standardize Null Values

Converts all null representations to empty strings.

- Click "Standardize Null Values"
- All recognized null values become empty cells
- Ensures consistency for downstream processing

## Downloading Results

After any cleaning operation:

1. Click "Download" button
2. Cleaned CSV file downloads automatically
3. Original data is not modified

## Understanding Results

Each operation returns a JSON result showing:

```json
{
  "message": "Operation description",
  "count": 5,
  "mode": "wasm"
}
```

- **message**: What was done
- **count**: Number of items affected
- **mode**: "wasm" (offline) or "api" (online)

