#!/usr/bin/env python3

import json
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from pathlib import Path

def load_results():
    pandas_results = []
    toolkit_results = []

    try:
        with open('/tmp/pandas_benchmark_results.json', 'r') as f:
            data = json.load(f)
            pandas_results = data.get('results', [])
    except FileNotFoundError:
        print("Pandas results not found")

    try:
        with open('/tmp/toolkit_benchmark_results.json', 'r') as f:
            data = json.load(f)
            toolkit_results = data.get('results', [])
    except FileNotFoundError:
        print("Toolkit results not found")

    return pandas_results + toolkit_results

def create_execution_time_comparison(results):
    if not results:
        print("No results to visualize")
        return

    datasets = set(r['dataset'] for r in results)
    tools = set(r['tool'] for r in results)

    fig, axes = plt.subplots(1, len(datasets), figsize=(15, 5))
    if len(datasets) == 1:
        axes = [axes]

    for idx, dataset in enumerate(sorted(datasets)):
        dataset_results = [r for r in results if r['dataset'] == dataset]

        tools_list = sorted(set(r['tool'] for r in dataset_results))
        means = [next((r['mean_ms'] for r in dataset_results if r['tool'] == t), 0) for t in tools_list]
        medians = [next((r['median_ms'] for r in dataset_results if r['tool'] == t), 0) for t in tools_list]

        x = range(len(tools_list))
        width = 0.35

        ax = axes[idx]
        ax.bar([i - width/2 for i in x], means, width, label='Mean', color='steelblue')
        ax.bar([i + width/2 for i in x], medians, width, label='Median', color='orange')

        ax.set_ylabel('Time (ms)')
        ax.set_title(f'{dataset} Dataset')
        ax.set_xticks(x)
        ax.set_xticklabels(tools_list)
        ax.legend()
        ax.grid(axis='y', alpha=0.3)

    plt.tight_layout()
    plt.savefig('/tmp/execution_time_comparison.png', dpi=300, bbox_inches='tight')
    print("Chart saved: /tmp/execution_time_comparison.png")

def create_accuracy_comparison(results):
    if not results:
        return

    datasets = set(r['dataset'] for r in results)
    fig, axes = plt.subplots(1, len(datasets), figsize=(15, 5))
    if len(datasets) == 1:
        axes = [axes]

    for idx, dataset in enumerate(sorted(datasets)):
        dataset_results = [r for r in results if r['dataset'] == dataset]

        tools_list = sorted(set(r['tool'] for r in dataset_results))
        accuracies = [next((r['accuracy'] for r in dataset_results if r['tool'] == t), 0) for t in tools_list]

        ax = axes[idx]
        colors = ['green' if acc >= 0.95 else 'orange' if acc >= 0.85 else 'red' for acc in accuracies]
        ax.bar(tools_list, accuracies, color=colors)

        ax.set_ylabel('Accuracy')
        ax.set_title(f'{dataset} Dataset - Accuracy')
        ax.set_ylim(0, 1.1)
        ax.grid(axis='y', alpha=0.3)

        for i, (tool, acc) in enumerate(zip(tools_list, accuracies)):
            ax.text(i, acc + 0.02, f'{acc:.4f}', ha='center', va='bottom')

    plt.tight_layout()
    plt.savefig('/tmp/accuracy_comparison.png', dpi=300, bbox_inches='tight')
    print("Chart saved: /tmp/accuracy_comparison.png")

def create_throughput_comparison(results):
    if not results:
        return

    datasets = set(r['dataset'] for r in results)
    fig, axes = plt.subplots(1, len(datasets), figsize=(15, 5))
    if len(datasets) == 1:
        axes = [axes]

    for idx, dataset in enumerate(sorted(datasets)):
        dataset_results = [r for r in results if r['dataset'] == dataset]

        tools_list = sorted(set(r['tool'] for r in dataset_results))
        throughputs = []
        for t in tools_list:
            result = next((r for r in dataset_results if r['tool'] == t), None)
            if result and result['mean_ms'] > 0:
                throughput = (result['rows_processed'] / result['mean_ms']) * 1000
                throughputs.append(throughput)
            else:
                throughputs.append(0)

        ax = axes[idx]
        ax.bar(tools_list, throughputs, color='purple')

        ax.set_ylabel('Rows/Second')
        ax.set_title(f'{dataset} Dataset - Throughput')
        ax.grid(axis='y', alpha=0.3)

        for i, (tool, tp) in enumerate(zip(tools_list, throughputs)):
            ax.text(i, tp + 100, f'{tp:.0f}', ha='center', va='bottom')

    plt.tight_layout()
    plt.savefig('/tmp/throughput_comparison.png', dpi=300, bbox_inches='tight')
    print("Chart saved: /tmp/throughput_comparison.png")

def create_statistical_summary_table(results):
    if not results:
        return

    print("\n" + "="*100)
    print("STATISTICAL SUMMARY TABLE")
    print("="*100 + "\n")

    print(f"{'Tool':<15} {'Dataset':<12} {'Task':<15} {'Mean':<10} {'Median':<10} {'StDev':<10} {'Accuracy':<10} {'Throughput':<12}")
    print("-" * 100)

    for result in sorted(results, key=lambda x: (x['tool'], x['dataset'])):
        throughput = (result['rows_processed'] / result['mean_ms']) * 1000 if result['mean_ms'] > 0 else 0
        print(f"{result['tool']:<15} {result['dataset']:<12} {result['task']:<15} "
              f"{result['mean_ms']:<10.2f} {result['median_ms']:<10.2f} {result['stdev_ms']:<10.2f} "
              f"{result['accuracy']:<10.4f} {throughput:<12.0f}")

if __name__ == "__main__":
    results = load_results()

    if results:
        print("\nGenerating visualizations...")
        create_execution_time_comparison(results)
        create_accuracy_comparison(results)
        create_throughput_comparison(results)
        create_statistical_summary_table(results)
        print("\nAll visualizations generated successfully.")
    else:
        print("No results found. Run benchmark_master.py first.")
