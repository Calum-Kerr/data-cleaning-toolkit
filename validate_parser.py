#!/usr/bin/env python3
import csv
import os
import sys

def validate_csv_files():
    datasets_dir = "datasets"
    test_cases = []
    passed = 0
    failed = 0
    for root, dirs, files in os.walk(datasets_dir):
        for file in files:
            if file.endswith(".csv"):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r', encoding='utf-8') as f:
                        reader = csv.reader(f)
                        rows = list(reader)
                        if len(rows) > 0:
                            passed += 1
                            print(f"✓ {filepath} ({len(rows)} rows, {len(rows[0])} cols)")
                except Exception as e:
                    failed += 1
                    print(f"✗ {filepath}: {e}")
    print(f"\nValidation: {passed} passed, {failed} failed")
    return failed == 0

if __name__ == "__main__":
    success = validate_csv_files()
    sys.exit(0 if success else 1)
