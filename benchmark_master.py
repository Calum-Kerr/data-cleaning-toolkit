#!/usr/bin/env python3

import json
import sys
from pathlib import Path
from benchmark_framework import BenchmarkFramework, BenchmarkResult
from benchmark_pandas import PandasBenchmark

def load_results_json(filepath: str) -> list:
    try:
        with open(filepath, 'r') as f:
            data = json.load(f)
            return data.get('results', [])
    except FileNotFoundError:
        return []

def generate_comparative_analysis():
    print("\n" + "="*90)
    print("COMPREHENSIVE BENCHMARK COMPARATIVE ANALYSIS")
    print("="*90 + "\n")

    pandas_results = load_results_json('/tmp/pandas_benchmark_results.json')
    toolkit_results = load_results_json('/tmp/toolkit_benchmark_results.json')

    all_results = pandas_results + toolkit_results

    if not all_results:
        print("ERROR: No benchmark results found. Run both benchmark scripts first.")
        return

    # Comparative execution time table
    print("\n1. EXECUTION TIME COMPARISON\n")
    print(f"{'Tool':<15} {'Dataset':<12} {'Task':<18} {'Mean (ms)':<12} {'Median (ms)':<12} {'StDev':<10}")
    print("-" * 90)

    for result in all_results:
        print(f"{result['tool']:<15} {result['dataset']:<12} {result['task']:<18} "
              f"{result['mean_ms']:<12.2f} {result['median_ms']:<12.2f} {result['stdev_ms']:<10.2f}")

    # Accuracy comparison
    print("\n\n2. ACCURACY COMPARISON\n")
    print(f"{'Tool':<15} {'Dataset':<12} {'Task':<18} {'Accuracy':<15} {'Errors Detected':<15}")
    print("-" * 75)

    for result in all_results:
        print(f"{result['tool']:<15} {result['dataset']:<12} {result['task']:<18} "
              f"{result['accuracy']:<15.4f} {result['errors_detected']:<15}")

    # Throughput analysis
    print("\n\n3. THROUGHPUT ANALYSIS (Rows per second)\n")
    print(f"{'Tool':<15} {'Dataset':<12} {'Task':<18} {'Rows/Sec':<15}")
    print("-" * 60)

    for result in all_results:
        if result['mean_ms'] > 0:
            throughput = (result['rows_processed'] / result['mean_ms']) * 1000
            print(f"{result['tool']:<15} {result['dataset']:<12} {result['task']:<18} {throughput:<15.0f}")

    # Audit log volume (if available)
    print("\n\n4. AUDIT LOG VOLUME\n")
    print(f"{'Tool':<15} {'Dataset':<12} {'Task':<18} {'Log Lines':<15}")
    print("-" * 60)

    for result in all_results:
        print(f"{result['tool']:<15} {result['dataset']:<12} {result['task']:<18} {result['audit_log_lines']:<15}")

    # Statistical significance
    print("\n\n5. STATISTICAL SUMMARY\n")
    tools = set(r['tool'] for r in all_results)
    for tool in sorted(tools):
        tool_results = [r for r in all_results if r['tool'] == tool]
        print(f"\n{tool}:")
        print(f"  Total benchmarks: {len(tool_results)}")
        mean_times = [r['mean_ms'] for r in tool_results]
        print(f"  Average execution time: {sum(mean_times)/len(mean_times):.2f}ms")
        print(f"  Average accuracy: {sum(r['accuracy'] for r in tool_results)/len(tool_results):.4f}")

if __name__ == "__main__":
    framework = BenchmarkFramework("/Users/ck/Desktop/controlled-refactor/data-cleaning-toolkit/datasets")

    print("\n" + "="*90)
    print("EXPERIMENTAL SETUP")
    print("="*90)
    framework.print_experiment_plan()

    print("\n" + "="*90)
    print("STEP 1: Running Pandas Benchmarks...")
    print("="*90)

    pandas_bench = PandasBenchmark("/Users/ck/Desktop/controlled-refactor/data-cleaning-toolkit/datasets")

    results = []
    results.append(pandas_bench.benchmark_duplicates("Airbnb"))
    results.append(pandas_bench.benchmark_missing_values("Titanic"))
    results.append(pandas_bench.benchmark_outliers("Credit"))

    with open('/tmp/pandas_benchmark_results.json', 'w') as f:
        json.dump({"results": [r.get_statistics() for r in results]}, f, indent=2)

    print("\nPandas benchmarks complete. Results saved.")

    print("\n" + "="*90)
    print("STEP 2: Toolkit Benchmarks")
    print("="*90)
    print("\nTo benchmark the toolkit:")
    print("1. Start the toolkit on your server: ./Toolkit")
    print("2. Run: python3 benchmark_toolkit.py")
    print("3. Run this script again to generate comparative analysis")

    # Try to load toolkit results if available
    toolkit_results = load_results_json('/tmp/toolkit_benchmark_results.json')
    if toolkit_results:
        print("\nToolkit results found. Generating comparative analysis...")
        generate_comparative_analysis()
    else:
        print("\nNo toolkit results yet. Run benchmark_toolkit.py on server after starting the toolkit.")
