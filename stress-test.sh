#!/bin/bash

# Stress Testing Script
API_URL="http://localhost:8080"

echo "=========================================="
echo "Stress Testing - Large Data & Edge Cases"
echo "=========================================="

# Generate large CSV file (1000 rows)
echo "Generating large CSV (1000 rows)..."
{
    echo "id,name,email,city,status"
    for i in {1..1000}; do
        echo "$i,User$i,user$i@example.com,City$((i % 50)),Active"
    done
} > test-data/large-1000.csv

echo "Testing with 1000 rows..."
time curl -s -X POST "$API_URL/api/parse" -d @test-data/large-1000.csv | head -c 100
echo ""

# Generate very large CSV (5000 rows)
echo "Generating very large CSV (5000 rows)..."
{
    echo "id,name,email,city,status"
    for i in {1..5000}; do
        echo "$i,User$i,user$i@example.com,City$((i % 50)),Active"
    done
} > test-data/large-5000.csv

echo "Testing with 5000 rows..."
time curl -s -X POST "$API_URL/api/parse" -d @test-data/large-5000.csv | head -c 100
echo ""

# Test with many columns (50 columns)
echo "Generating CSV with 50 columns..."
{
    header="id"
    for i in {1..49}; do
        header="$header,col$i"
    done
    echo "$header"
    for i in {1..100}; do
        row="$i"
        for j in {1..49}; do
            row="$row,value$i$j"
        done
        echo "$row"
    done
} > test-data/wide-50cols.csv

echo "Testing with 50 columns..."
time curl -s -X POST "$API_URL/api/parse" -d @test-data/wide-50cols.csv | head -c 100
echo ""

# Test with special characters
echo "Generating CSV with special characters..."
{
    echo "id,name,description"
    echo "1,Test™,Unicode™ test"
    echo "2,Café,Accented café"
    echo "3,Quote\"Test,Contains\"quotes"
    echo "4,Comma,Test,with,commas"
    echo "5,Newline,Test with newline"
} > test-data/special-chars.csv

echo "Testing with special characters..."
curl -s -X POST "$API_URL/api/parse" -d @test-data/special-chars.csv | head -c 100
echo ""

# Test with empty file
echo "Testing with empty file..."
curl -s -X POST "$API_URL/api/parse" -d "" | head -c 100
echo ""

# Test with single row
echo "Testing with single row..."
curl -s -X POST "$API_URL/api/parse" -d "id,name,email" | head -c 100
echo ""

# Test rapid sequential requests
echo "Testing rapid sequential requests (10 requests)..."
for i in {1..10}; do
    curl -s -X POST "$API_URL/api/health" > /dev/null
    echo -n "."
done
echo " Done"

echo ""
echo "=========================================="
echo "Stress Testing Complete"
echo "=========================================="

