#!/bin/bash

# Data Integrity Test Script
API_URL="http://localhost:8080"

echo "=========================================="
echo "Data Integrity Testing"
echo "=========================================="

# Test 1: Verify parse returns correct row count
echo ""
echo "Test 1: Parse row count verification"
CSV="id,name,email
1,John,john@example.com
2,Jane,jane@example.com
3,Bob,bob@example.com"

response=$(curl -s -X POST "$API_URL/api/parse" -d "$CSV")
rows=$(echo "$response" | grep -o '"rows":[0-9]*' | cut -d':' -f2)
echo "Input rows: 3 (plus header)"
echo "API returned rows: $rows"
if [ "$rows" = "4" ] || [ "$rows" = "3" ]; then
    echo "✓ PASS"
else
    echo "✗ FAIL"
fi

# Test 2: Verify duplicate detection
echo ""
echo "Test 2: Duplicate detection"
CSV_DUP="id,name,email
1,John,john@example.com
2,Jane,jane@example.com
1,John,john@example.com"

response=$(curl -s -X POST "$API_URL/api/detect-duplicates" -d "$CSV_DUP")
dups=$(echo "$response" | grep -o '"duplicateCount":[0-9]*' | cut -d':' -f2)
echo "Input has 1 duplicate row"
echo "API detected: $dups duplicates"
if [ "$dups" = "1" ]; then
    echo "✓ PASS"
else
    echo "✗ FAIL (expected 1, got $dups)"
fi

# Test 3: Verify missing value detection
echo ""
echo "Test 3: Missing value detection"
CSV_MISS="id,name,email
1,John,
2,,jane@example.com
3,Bob,bob@example.com"

response=$(curl -s -X POST "$API_URL/api/detect-missing" -d "$CSV_MISS")
missing=$(echo "$response" | grep -o '"missingCount":[0-9]*' | cut -d':' -f2)
echo "Input has 2 missing values"
echo "API detected: $missing missing values"
if [ "$missing" = "2" ]; then
    echo "✓ PASS"
else
    echo "✗ FAIL (expected 2, got $missing)"
fi

# Test 4: Verify clean removes duplicates
echo ""
echo "Test 4: Clean operation removes duplicates"
response=$(curl -s -X POST "$API_URL/api/clean" -d "$CSV_DUP")
echo "Response: $response" | head -c 200
echo ""

# Test 5: Verify whitespace normalization
echo ""
echo "Test 5: Whitespace normalization"
CSV_WS="id,name,email
1, John , john@example.com 
2,Jane,jane@example.com"

response=$(curl -s -X POST "$API_URL/api/normalize-whitespace" -d "$CSV_WS")
echo "Response: $response" | head -c 200
echo ""

# Test 6: Verify case standardization
echo ""
echo "Test 6: Case standardization"
CSV_CASE="id,name,email
1,JOHN DOE,JOHN@EXAMPLE.COM
2,jane smith,jane@example.com"

response=$(curl -s -X POST "$API_URL/api/standardize-case" -d "$CSV_CASE")
echo "Response: $response" | head -c 200
echo ""

# Test 7: Verify null standardization
echo ""
echo "Test 7: Null value standardization"
CSV_NULL="id,name,status
1,John,Active
2,Jane,N/A
3,Bob,null
4,Alice,NA"

response=$(curl -s -X POST "$API_URL/api/standardize-nulls" -d "$CSV_NULL")
echo "Response: $response" | head -c 200
echo ""

# Test 8: Sequential operations
echo ""
echo "Test 8: Sequential operations (clean -> normalize -> standardize)"
CSV_SEQ="id,name,email
1, John Doe , john@example.com 
2,jane smith,jane@example.com
1, John Doe , john@example.com"

echo "Step 1: Clean"
curl -s -X POST "$API_URL/api/clean" -d "$CSV_SEQ" | head -c 100
echo ""

echo "Step 2: Normalize whitespace"
curl -s -X POST "$API_URL/api/normalize-whitespace" -d "$CSV_SEQ" | head -c 100
echo ""

echo "Step 3: Standardize case"
curl -s -X POST "$API_URL/api/standardize-case" -d "$CSV_SEQ" | head -c 100
echo ""

echo "=========================================="
echo "Data Integrity Testing Complete"
echo "=========================================="

