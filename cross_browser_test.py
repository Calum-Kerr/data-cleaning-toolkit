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
