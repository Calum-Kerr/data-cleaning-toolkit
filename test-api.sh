#!/bin/bash

# Test API endpoints
API_URL="http://localhost:8080"
TEST_CSV="id,name,email
1,John Doe,john@example.com
2,Jane Smith,jane@example.com
1,John Doe,john@example.com"

echo "Testing Data Cleaning Toolkit API"
echo "=================================="
echo ""

# Test health endpoint
echo "1. Testing /api/health"
curl -s "$API_URL/api/health" | jq . || echo "FAILED"
echo ""

# Test parse endpoint
echo "2. Testing /api/parse"
curl -s -X POST "$API_URL/api/parse" -d "$TEST_CSV" | jq . || echo "FAILED"
echo ""

# Test detect-duplicates
echo "3. Testing /api/detect-duplicates"
curl -s -X POST "$API_URL/api/detect-duplicates" -d "$TEST_CSV" | jq . || echo "FAILED"
echo ""

# Test detect-missing
echo "4. Testing /api/detect-missing"
curl -s -X POST "$API_URL/api/detect-missing" -d "$TEST_CSV" | jq . || echo "FAILED"
echo ""

# Test clean
echo "5. Testing /api/clean"
curl -s -X POST "$API_URL/api/clean" -d "$TEST_CSV" | jq . || echo "FAILED"
echo ""

# Test normalize-whitespace
echo "6. Testing /api/normalize-whitespace"
curl -s -X POST "$API_URL/api/normalize-whitespace" -d "$TEST_CSV" | jq . || echo "FAILED"
echo ""

# Test standardize-case
echo "7. Testing /api/standardize-case"
curl -s -X POST "$API_URL/api/standardize-case" -d "$TEST_CSV" | jq . || echo "FAILED"
echo ""

echo "API Testing Complete"

