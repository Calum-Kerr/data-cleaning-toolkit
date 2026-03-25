#!/usr/bin/env python3
"""
Comprehensive API Testing Suite
Tests all endpoints with various inputs and validates responses
"""

import requests
import json
import time
import sys
from typing import Dict, Any, Tuple

API_URL = "http://localhost:8080"
PASS = 0
FAIL = 0
TOTAL = 0

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'

def test_endpoint(name: str, method: str, endpoint: str, data: str = None) -> Tuple[bool, str]:
    """Test a single endpoint and return result"""
    global PASS, FAIL, TOTAL
    TOTAL += 1
    
    try:
        url = f"{API_URL}{endpoint}"
        start = time.time()
        
        if method == "GET":
            response = requests.get(url, timeout=5)
        else:
            response = requests.post(url, data=data, timeout=5)
        
        elapsed = time.time() - start
        
        # Validate response
        try:
            json_data = response.json()
            is_valid_json = True
        except:
            is_valid_json = False
        
        success = response.status_code == 200 and is_valid_json
        
        if success:
            PASS += 1
            status = f"{Colors.GREEN}✓ PASS{Colors.END}"
        else:
            FAIL += 1
            status = f"{Colors.RED}✗ FAIL{Colors.END}"
        
        print(f"\n{Colors.YELLOW}Test {TOTAL}: {name}{Colors.END}")
        print(f"  Status: {response.status_code} | Time: {elapsed:.3f}s")
        print(f"  Result: {status}")
        if is_valid_json:
            print(f"  Response: {str(json_data)[:100]}...")
        else:
            print(f"  Response: {response.text[:100]}...")
        
        return success, response.text
        
    except Exception as e:
        FAIL += 1
        print(f"\n{Colors.YELLOW}Test {TOTAL}: {name}{Colors.END}")
        print(f"  {Colors.RED}✗ FAIL - Exception: {str(e)}{Colors.END}")
        return False, str(e)

def main():
    print(f"\n{Colors.BLUE}{'='*50}")
    print("Comprehensive API Testing Suite")
    print(f"{'='*50}{Colors.END}\n")
    
    # Test data
    csv_basic = "id,name,email\n1,John,john@example.com\n2,Jane,jane@example.com\n1,John,john@example.com"
    csv_missing = "id,name,email\n1,John,\n2,,jane@example.com\n3,Bob,bob@example.com"
    csv_whitespace = "id,name,email\n1, John , john@example.com \n2,Jane,jane@example.com"
    csv_nulls = "id,name,status\n1,John,Active\n2,Jane,N/A\n3,Bob,null"
    
    # Test 1: Health Check
    test_endpoint("Health Check", "GET", "/api/health")
    
    # Test 2-4: Parse endpoint
    test_endpoint("Parse Basic CSV", "POST", "/api/parse", csv_basic)
    test_endpoint("Parse Empty CSV", "POST", "/api/parse", "")
    test_endpoint("Parse Single Row", "POST", "/api/parse", "id,name,email")
    
    # Test 5-7: Detection endpoints
    test_endpoint("Detect Duplicates", "POST", "/api/detect-duplicates", csv_basic)
    test_endpoint("Detect Missing Values", "POST", "/api/detect-missing", csv_missing)
    test_endpoint("Detect Outliers", "POST", "/api/detect-outliers", csv_basic)
    
    # Test 8-12: Cleaning endpoints
    test_endpoint("Clean Data", "POST", "/api/clean", csv_basic)
    test_endpoint("Normalize Whitespace", "POST", "/api/normalize-whitespace", csv_whitespace)
    test_endpoint("Standardize Case", "POST", "/api/standardize-case", csv_basic)
    test_endpoint("Standardize Nulls", "POST", "/api/standardize-nulls", csv_nulls)
    test_endpoint("Remove Outliers", "POST", "/api/remove-outliers", csv_basic)
    
    # Test 13-14: Additional endpoints
    test_endpoint("Remove State Suffixes", "POST", "/api/remove-state-suffixes", csv_basic)
    test_endpoint("Remove Duplicate Words", "POST", "/api/remove-duplicate-words", csv_basic)
    
    # Print summary
    print(f"\n{Colors.BLUE}{'='*50}")
    print(f"Results: {Colors.GREEN}{PASS} PASSED{Colors.END}, {Colors.RED}{FAIL} FAILED{Colors.END}, Total: {TOTAL}")
    print(f"{'='*50}{Colors.END}\n")
    
    return 0 if FAIL == 0 else 1

if __name__ == "__main__":
    sys.exit(main())

