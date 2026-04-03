#!/bin/bash
# Generate dissertation-quality visualizations from benchmark results

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
RESULTS_FILE="$SCRIPT_DIR/benchmark_results/openrefine_comparison_results.json"
OUTPUT_DIR="$SCRIPT_DIR/benchmark_results"

echo "=========================================="
echo "Dissertation Visualization Generator"
echo "=========================================="
echo ""

# Check if results exist
if [ ! -f "$RESULTS_FILE" ]; then
    echo "Error: Results file not found at $RESULTS_FILE"
    echo "Run the benchmarks first:"
    echo "  ./run_openrefine_benchmark.sh"
    exit 1
fi

# Check Python
if ! command -v python3 &> /dev/null; then
    echo "Error: python3 not found"
    exit 1
fi

# Install matplotlib if needed
python3 -c "import matplotlib" 2>/dev/null || {
    echo "Installing matplotlib..."
    pip install matplotlib numpy
}

echo "Generating visualizations..."
echo "Input: $RESULTS_FILE"
echo "Output: $OUTPUT_DIR"
echo ""

python3 "$SCRIPT_DIR/visualize_openrefine_comparison.py" \
    --results "$RESULTS_FILE" \
    --output "$OUTPUT_DIR"

echo ""
echo "=========================================="
echo "Visualizations complete!"
echo "=========================================="
echo ""
echo "Generated images (ready for LaTeX):"
ls -lh "$OUTPUT_DIR"/*.png 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
echo ""
echo "See VISUALIZATION_GUIDE.md for LaTeX integration."
