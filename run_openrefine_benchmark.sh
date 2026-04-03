#!/bin/bash
# Run OpenRefine comparison benchmarks
# Usage: ./run_openrefine_benchmark.sh [--limit N] [--api-url URL]

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
API_URL="http://localhost:8080"
LIMIT=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --limit)
            LIMIT="--limit $2"
            shift 2
            ;;
        --api-url)
            API_URL="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "=========================================="
echo "Data Cleaning Toolkit vs OpenRefine"
echo "Comparison Benchmark"
echo "=========================================="
echo ""
echo "API URL: $API_URL"
echo "Datasets: $SCRIPT_DIR/datasets"
echo "Output: $SCRIPT_DIR/benchmark_results"
echo ""

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "Error: python3 not found"
    exit 1
fi

# Check if requests library is available
python3 -c "import requests" 2>/dev/null || {
    echo "Installing required libraries..."
    pip install requests
}

# Run benchmark
echo "Starting benchmark..."
python3 "$SCRIPT_DIR/benchmark_openrefine_comparison.py" \
    --api-url "$API_URL" \
    --datasets "$SCRIPT_DIR/datasets" \
    $LIMIT

echo ""
echo "Benchmark complete!"
echo "Results: $SCRIPT_DIR/benchmark_results/"
