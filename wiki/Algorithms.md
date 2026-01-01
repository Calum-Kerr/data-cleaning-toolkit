# Algorithms

All algorithms are implemented from scratch in C++ without external libraries.

## CSV Parser

Parses comma-separated values into a 2D data structure.

**Algorithm:**
1. Split input by newline characters to get rows
2. For each row, split by comma to get cells
3. Store in vector of vectors

**Complexity:** O(n) where n is the number of characters

**Limitations:**
- Does not handle quoted fields with commas inside
- Does not handle escaped characters
- Assumes consistent column count

## Missing Value Detection

Detects empty cells in the parsed data.

**Algorithm:**
1. Iterate through all rows
2. For each row, iterate through all cells
3. If cell is empty string, increment counter

**Complexity:** O(r × c) where r is rows and c is columns

## Duplicate Detection

Detects rows that appear more than once.

**Algorithm:**
1. Create a set to store seen rows
2. For each row, create a string key by joining cells
3. If key already in set, increment duplicate counter
4. Otherwise, add key to set

**Complexity:** O(r × c) average case using hash set

## Duplicate Removal

Removes duplicate rows, keeping first occurrence.

**Algorithm:**
1. Create a set to track seen rows
2. Create output vector for unique rows
3. For each row:
   - Create string key by joining cells
   - If key not in set, add row to output and key to set
   - If key in set, skip row (duplicate)

**Complexity:** O(r × c) average case

## Whitespace Detection

Detects cells with leading or trailing whitespace.

**Algorithm:**
1. For each cell, compare original to trimmed version
2. If different, cell has whitespace issues

**Complexity:** O(r × c × s) where s is average string length

## Whitespace Trimming

Removes leading and trailing whitespace from all cells.

**Algorithm:**
1. For each cell:
   - Find first non-whitespace character
   - Find last non-whitespace character
   - Extract substring between them

**Complexity:** O(r × c × s)

## Case Standardization

Converts all text to uppercase or lowercase.

**Algorithm:**
1. For each cell, for each character:
   - If lowercase and converting to upper: subtract 32 from ASCII value
   - If uppercase and converting to lower: add 32 to ASCII value

**Complexity:** O(r × c × s)

## Null Value Detection

Detects various representations of null/missing values.

**Algorithm:**
1. Define set of null representations: "", "N/A", "n/a", "NA", "null", "NULL", "None", "NONE", "-", "?"
2. For each cell, check if value matches any null representation

**Complexity:** O(r × c × k) where k is number of null representations

## Null Value Standardization

Converts all null representations to empty strings.

**Algorithm:**
1. For each cell:
   - If matches null representation, replace with empty string
   - Otherwise, keep original value

**Complexity:** O(r × c × k)

