# OpenRefine vs Data Cleaning Toolkit Comparison

Benchmarks comparing the accuracy, performance, and effectiveness of the Data Cleaning Toolkit against OpenRefine (industry standard).

## Files

- `openrefine_comparison_results.json` - Raw benchmark data (tool, dataset, performance metrics)
- `openrefine_comparison_results.csv` - Same data in CSV format for spreadsheet analysis
- `openrefine_comparison_report.json` - Summary statistics and comparison analysis

## Metrics

### Performance
- **mean_time_ms** - Average execution time across datasets
- **median_time_ms** - Median execution time
- **min/max_time_ms** - Fastest/slowest dataset

### Effectiveness
- **bytes_removed** - Total data reduced (measure of cleaning intensity)
- **rows_removed** - Number of rows cleaned/deduplicated
- **avg_bytes_removed** - Average bytes cleaned per dataset
- **avg_rows_removed** - Average rows cleaned per dataset

## How to Run

```bash
# Basic run (tests all 103 datasets)
python3 benchmark_openrefine_comparison.py

# Specify custom API endpoint
python3 benchmark_openrefine_comparison.py --api-url http://your-server:8080

# Limit to N datasets (useful for quick testing)
python3 benchmark_openrefine_comparison.py --limit 10

# Custom datasets directory
python3 benchmark_openrefine_comparison.py --datasets /path/to/datasets
```

## Requirements

- Python 3.7+
- `requests` library (for API calls): `pip install requests`
- Running Data Cleaning Toolkit server on localhost:8080

## Results Interpretation

### Toolkit Advantages
- Shows performance (lower time_ms = faster)
- Shows cleaning effectiveness (higher bytes_removed = more aggressive cleaning)
- Measured on real 103 production datasets

### Comparison Insights
- **Speed**: Toolkit should be faster due to C++ backend with WASM frontend
- **Accuracy**: Both should achieve similar deduplication and null handling
- **Coverage**: Toolkit handles JSON/XML; OpenRefine is CSV-focused

## Data Integrity

- All test datasets remain unchanged
- Results are append-only (new runs add to existing results)
- No modifications to input files
