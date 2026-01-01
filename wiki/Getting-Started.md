# Getting Started

This guide will help you start using the Data Cleaning Toolkit.

## Using the Live Demo

The easiest way to use the toolkit is through the live demo:

1. Go to https://www.calumkerr.com/app
2. Upload a CSV file using the file input
3. Click detection buttons to find data quality issues
4. Click cleaning buttons to fix issues
5. Download the cleaned CSV

## Supported Features

### Detection
- **Detect Missing Values**: Finds empty cells in your data
- **Detect Duplicates**: Finds duplicate rows
- **Detect Whitespace**: Finds leading/trailing whitespace in cells
- **Detect Null Values**: Finds various null representations (N/A, null, -, etc.)

### Cleaning
- **Remove Duplicates**: Removes duplicate rows, keeps first occurrence
- **Trim Whitespace**: Removes leading/trailing whitespace from all cells
- **Standardize Case**: Converts text to uppercase or lowercase
- **Standardize Null Values**: Converts all null representations to empty strings

## Offline Mode

The toolkit works completely offline after the first load:

1. Visit the demo once while connected to the internet
2. The service worker caches all necessary files
3. You can now use the toolkit without an internet connection
4. Your data never leaves your device in offline mode

## Sample Data

Try with this sample CSV to test the features:

```csv
name,age,city,notes
John,25,Edinburgh,
Derek,N/A,Glasgow,null
  Sarah  ,30,London,test
John,25,Edinburgh,
```

This sample contains:
- Missing values (empty notes)
- Null representations (N/A, null)
- Whitespace issues (Sarah has spaces)
- Duplicate rows (John appears twice)

