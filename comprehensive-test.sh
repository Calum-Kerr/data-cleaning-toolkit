#!/bin/bash

# Comprehensive API Testing Script
API_URL="http://localhost:8080"
PASS=0
FAIL=0
TOTAL=0

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

test_endpoint() {
    local name=$1
    local method=$2
    local endpoint=$3
    local data=$4
    local expected_status=$5
    
    TOTAL=$((TOTAL + 1))
    echo -e "\n${YELLOW}Test $TOTAL: $name${NC}"
    
    if [ "$method" = "GET" ]; then
        response=$(curl -s -w "\n%{http_code}" "$API_URL$endpoint")
    else
        response=$(curl -s -w "\n%{http_code}" -X POST "$API_URL$endpoint" -d "$data")
    fi
    
    status=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n-1)
    
    if [ "$status" = "$expected_status" ]; then
        echo -e "${GREEN}✓ PASS${NC} (Status: $status)"
        echo "Response: $body" | head -c 200
        echo ""
        PASS=$((PASS + 1))
    else
        echo -e "${RED}✗ FAIL${NC} (Expected: $expected_status, Got: $status)"
        echo "Response: $body" | head -c 200
        echo ""
        FAIL=$((FAIL + 1))
    fi
}

echo "=========================================="
echo "Comprehensive API Testing"
echo "=========================================="

# Test 1: Health Check
test_endpoint "Health Check" "GET" "/api/health" "" "200"

# Test 2: Parse Basic CSV
CSV_BASIC=$(cat test-data/test-basic.csv)
test_endpoint "Parse Basic CSV" "POST" "/api/parse" "$CSV_BASIC" "200"

# Test 3: Parse Empty CSV
test_endpoint "Parse Empty CSV" "POST" "/api/parse" "" "200"

# Test 4: Detect Duplicates
test_endpoint "Detect Duplicates" "POST" "/api/detect-duplicates" "$CSV_BASIC" "200"

# Test 5: Detect Missing Values
CSV_MISSING=$(cat test-data/test-missing.csv)
test_endpoint "Detect Missing Values" "POST" "/api/detect-missing" "$CSV_MISSING" "200"

# Test 6: Clean Data (Remove Duplicates)
test_endpoint "Clean Data" "POST" "/api/clean" "$CSV_BASIC" "200"

# Test 7: Normalize Whitespace
CSV_WS=$(cat test-data/test-whitespace.csv)
test_endpoint "Normalize Whitespace" "POST" "/api/normalize-whitespace" "$CSV_WS" "200"

# Test 8: Standardize Case
test_endpoint "Standardize Case" "POST" "/api/standardize-case" "$CSV_BASIC" "200"

# Test 9: Standardize Nulls
CSV_NULLS=$(cat test-data/test-nulls.csv)
test_endpoint "Standardize Nulls" "POST" "/api/standardize-nulls" "$CSV_NULLS" "200"

# Test 10: Detect Outliers
test_endpoint "Detect Outliers" "POST" "/api/detect-outliers" "$CSV_BASIC" "200"

# Test 11: Remove Outliers
test_endpoint "Remove Outliers" "POST" "/api/remove-outliers" "$CSV_BASIC" "200"

# Test 12: Remove State Suffixes
test_endpoint "Remove State Suffixes" "POST" "/api/remove-state-suffixes" "$CSV_BASIC" "200"

# Test 13: Remove Duplicate Words
test_endpoint "Remove Duplicate Words" "POST" "/api/remove-duplicate-words" "$CSV_BASIC" "200"

echo ""
echo "=========================================="
echo -e "Results: ${GREEN}$PASS PASSED${NC}, ${RED}$FAIL FAILED${NC}, Total: $TOTAL"
echo "=========================================="

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi

