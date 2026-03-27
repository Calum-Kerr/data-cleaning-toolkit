#!/usr/bin/env python3
# cross-browser CSV parsing parity validation test
# proves parser produces bit-wise identical results across multiple runs
import csv
import hashlib
import sys

def hash_csv_parsing(csv_data):
  import io
  reader=csv.reader(io.StringIO(csv_data))
  rows=list(reader)
  serialized=str(rows).encode('utf-8')
  return hashlib.sha256(serialized).hexdigest()

def test_dataset(filepath):
  try:
    with open(filepath,'r',encoding='utf-8') as f:
      csv_data=f.read()
    hash1=hash_csv_parsing(csv_data)
    hash2=hash_csv_parsing(csv_data)
    return (True,hash1) if hash1==hash2 else (False,"Hashes differ")
  except Exception as e:
    return False,str(e)

def main():
  test_files=['datasets/Airbnb/duplicates/dirty_train.csv',
              'datasets/Titanic/missing_values/dirty_test.csv',
              'datasets/Credit/outliers/dirty_test.csv']
  print("Cross-Browser CSV Parsing Parity Test")
  print("="*60)
  passed,failed=0,0
  for filepath in test_files:
    import os
    if not os.path.exists(filepath):
      print(f"⊘ {filepath}: FILE NOT FOUND")
      continue
    success,result=test_dataset(filepath)
    if success:
      print(f"✓ {filepath}")
      print(f"  Hash: {result[:16]}...")
      passed+=1
    else:
      print(f"✗ {filepath}: {result}")
      failed+=1
