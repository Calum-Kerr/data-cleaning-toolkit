#!/bin/bash

# Error Handling Test Script
API_URL="http://localhost:8080"
PASS=0
FAIL=0

echo "=========================================="
echo "Error Handling & Edge Case Testing"
echo "=========================================="

test_error() {
    local name=$1
    local endpoint=$2
    local data=$3
    
    echo ""
    echo "Testing: $name"
    response=$(curl -s -w "\n%{http_code}" -X POST "$API_URL$endpoint" -d "$data")
    status=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n-1)
    
    echo "Status: $status"
    echo "Response: $body" | head -c 150
    echo ""
    
    if [ "$status" != "000" ]; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
    fi
}

# Test 1: Malformed CSV (missing closing quote)
test_error "Malformed CSV - unclosed quote" "/api/parse" 'id,name,email
1,"John Doe,john@example.com'

# Test 2: Very long string
LONG_STRING=$(printf 'a%.0s' {1..10000})
test_error "Very long string" "/api/parse" "id,name
1,$LONG_STRING"

# Test 3: CSV with only headers
test_error "CSV with only headers" "/api/parse" "id,name,email"

# Test 4: CSV with inconsistent columns
test_error "Inconsistent columns" "/api/parse" "id,name,email
1,John,john@example.com
2,Jane,jane@example.com,extra"

# Test 5: CSV with null bytes
test_error "Null bytes in data" "/api/parse" "id,name
1,John$(printf '\0')Doe"

# Test 6: CSV with only whitespace
test_error "Only whitespace" "/api/parse" "   
   
   "

# Test 7: CSV with tabs
test_error "Tab-separated values" "/api/parse" "id	name	email
1	John	john@example.com"

# Test 8: Very large payload (simulate)
LARGE_PAYLOAD=$(printf 'id,name\n%.0s' {1..100})
for i in {1..100}; do
    LARGE_PAYLOAD="$LARGE_PAYLOAD$i,User$i\n"
done
test_error "Large payload" "/api/parse" "$LARGE_PAYLOAD"

# Test 9: Invalid endpoint
echo ""
echo "Testing: Invalid endpoint"
response=$(curl -s -w "\n%{http_code}" "$API_URL/api/nonexistent")
status=$(echo "$response" | tail -n1)
echo "Status: $status (should be 404)"

# Test 10: Wrong HTTP method
echo ""
echo "Testing: Wrong HTTP method (GET instead of POST)"
response=$(curl -s -w "\n%{http_code}" "$API_URL/api/clean")
status=$(echo "$response" | tail -n1)
echo "Status: $status (should be 405 or similar)"

echo ""
echo "=========================================="
echo "Error Testing Complete"
echo "=========================================="

