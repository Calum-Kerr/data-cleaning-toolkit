#!/usr/bin/env python3
"""
Generate dissertation-quality visualizations from real dataset benchmarks
Compares Reference Cleaner vs Toolkit across real CSV files
"""

import json
import csv
from pathlib import Path
from typing import Dict, List, Any
import statistics

try:
    import matplotlib.pyplot as plt
    import matplotlib.patches as mpatches
    import numpy as np
except ImportError:
    print("Error: matplotlib not found. Install with: pip install matplotlib numpy")
    exit(1)

# Dissertation-quality styling
plt.style.use('seaborn-v0_8-darkgrid')
COLORS = {
    'Reference Cleaner': '#0066cc',
    'Toolkit': '#00cc00',
}
FONT_SIZE = 11
plt.rcParams.update({
    'font.size': FONT_SIZE,
    'font.family': 'sans-serif',
    'axes.labelsize': FONT_SIZE + 1,
    'axes.titlesize': FONT_SIZE + 2,
    'xtick.labelsize': FONT_SIZE,
    'ytick.labelsize': FONT_SIZE,
    'legend.fontsize': FONT_SIZE,
    'figure.titlesize': FONT_SIZE + 3,
    'lines.linewidth': 2,
    'lines.markersize': 6
})

class RealBenchmarkVisualizer:
    def __init__(self, results_file: Path = None, output_dir: Path = None):
        self.results_file = results_file or (
            Path(__file__).parent / "benchmark_results" / "real_dataset_benchmark_results.json"
        )
        self.output_dir = output_dir or (Path(__file__).parent / "benchmark_results")
        self.output_dir.mkdir(exist_ok=True)
        self.results = []
        self.load_results()

    def load_results(self):
        """Load benchmark results from JSON"""
        if not self.results_file.exists():
            print(f"Error: Results file not found at {self.results_file}")
            exit(1)

        with open(self.results_file, 'r') as f:
            data = json.load(f)
            self.results = data.get("results", [])

        print(f"Loaded {len(self.results)} benchmark results")

    def performance_comparison(self):
        """Generate performance (execution time) comparison chart"""
        fig, ax = plt.subplots(figsize=(12, 6))

        by_tool = {}
        for r in self.results:
            if r.get("status") != "success":
                continue
            tool = r.get("tool")
            if tool not in by_tool:
                by_tool[tool] = []
            by_tool[tool].append(r.get("time_ms", 0))

        tools = list(by_tool.keys())
        times_data = [by_tool[tool] for tool in tools]

        bp = ax.boxplot(times_data, labels=tools, patch_artist=True)

        for patch, tool in zip(bp['boxes'], tools):
            patch.set_facecolor(COLORS.get(tool, '#cccccc'))

        ax.set_ylabel('Execution Time (ms)', fontsize=FONT_SIZE + 1)
        ax.set_title('Performance Comparison: Execution Time Distribution', fontsize=FONT_SIZE + 2)
        ax.grid(axis='y', alpha=0.3)

        # Add statistics
        for i, tool in enumerate(tools, 1):
            times = times_data[i-1]
            if times:
                mean = statistics.mean(times)
                median = statistics.median(times)
                ax.text(i, max(times) * 0.95, f'μ={mean:.0f}ms\nM={median:.0f}ms',
                       ha='center', fontsize=9, bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

        plt.tight_layout()
        output_path = self.output_dir / "performance_comparison.png"
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()

    def effectiveness_comparison(self):
        """Generate effectiveness (data reduction) comparison chart"""
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

        by_tool = {}
        for r in self.results:
            if r.get("status") != "success":
                continue
            tool = r.get("tool")
            if tool not in by_tool:
                by_tool[tool] = {"bytes": [], "rows": []}
            by_tool[tool]["bytes"].append(r.get("bytes_removed", 0))
            by_tool[tool]["rows"].append(r.get("rows_removed", 0))

        tools = list(by_tool.keys())
        x = np.arange(len(tools))
        width = 0.35

        # Bytes removed
        bytes_avg = [statistics.mean(by_tool[t]["bytes"]) for t in tools]
        bars1 = ax1.bar(x, bytes_avg, width, color=[COLORS.get(t, '#cccccc') for t in tools])
        ax1.set_ylabel('Average Bytes Removed', fontsize=FONT_SIZE + 1)
        ax1.set_title('Data Reduction: Bytes', fontsize=FONT_SIZE + 1)
        ax1.set_xticks(x)
        ax1.set_xticklabels(tools)
        ax1.grid(axis='y', alpha=0.3)

        # Add value labels on bars
        for bar in bars1:
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height,
                    f'{int(height):,}',
                    ha='center', va='bottom', fontsize=9)

        # Rows removed
        rows_avg = [statistics.mean(by_tool[t]["rows"]) for t in tools]
        bars2 = ax2.bar(x, rows_avg, width, color=[COLORS.get(t, '#cccccc') for t in tools])
        ax2.set_ylabel('Average Rows Removed', fontsize=FONT_SIZE + 1)
        ax2.set_title('Data Reduction: Rows', fontsize=FONT_SIZE + 1)
        ax2.set_xticks(x)
        ax2.set_xticklabels(tools)
        ax2.grid(axis='y', alpha=0.3)

        # Add value labels on bars
        for bar in bars2:
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.1f}',
                    ha='center', va='bottom', fontsize=9)

        plt.tight_layout()
        output_path = self.output_dir / "effectiveness_comparison.png"
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()

    def dataset_breakdown(self):
        """Generate dataset-by-dataset analysis"""
        by_dataset = {}
        for r in self.results:
            if r.get("status") != "success":
                continue
            ds = r.get("dataset", "Unknown")
            tool = r.get("tool")
            if ds not in by_dataset:
                by_dataset[ds] = {}
            if tool not in by_dataset[ds]:
                by_dataset[ds][tool] = {"times": [], "bytes": [], "rows": []}
            by_dataset[ds][tool]["times"].append(r.get("time_ms", 0))
            by_dataset[ds][tool]["bytes"].append(r.get("bytes_removed", 0))
            by_dataset[ds][tool]["rows"].append(r.get("rows_removed", 0))

        fig, ax = plt.subplots(figsize=(12, 6))

        datasets = sorted(by_dataset.keys())
        x = np.arange(len(datasets))
        width = 0.35

        tools = list(set(tool for ds in by_dataset.values() for tool in ds.keys()))

        for i, tool in enumerate(sorted(tools)):
            times_avg = []
            for ds in datasets:
                times = by_dataset[ds].get(tool, {}).get("times", [])
                times_avg.append(statistics.mean(times) if times else 0)

            ax.bar(x + i*width, times_avg, width, label=tool,
                   color=COLORS.get(tool, '#cccccc'))

        ax.set_ylabel('Average Execution Time (ms)', fontsize=FONT_SIZE + 1)
        ax.set_title('Performance Across Datasets', fontsize=FONT_SIZE + 2)
        ax.set_xticks(x + width/2)
        ax.set_xticklabels(datasets, rotation=45, ha='right')
        ax.legend()
        ax.grid(axis='y', alpha=0.3)

        plt.tight_layout()
        output_path = self.output_dir / "dataset_breakdown.png"
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()

    def summary_statistics(self):
        """Generate summary statistics table"""
        by_tool = {}
        for r in self.results:
            if r.get("status") != "success":
                continue
            tool = r.get("tool")
            if tool not in by_tool:
                by_tool[tool] = []
            by_tool[tool].append(r)

        fig, ax = plt.subplots(figsize=(12, 6))
        ax.axis('off')

        rows = [["Metric", "Reference Cleaner", "Toolkit"]]

        for tool in ["Reference Cleaner", "Toolkit"]:
            results = by_tool.get(tool, [])
            if not results:
                continue

            times = [r["time_ms"] for r in results]
            bytes_removed = [r["bytes_removed"] for r in results]
            rows_removed = [r["rows_removed"] for r in results]

            stats = {
                "Tests": len(results),
                "Avg Time (ms)": f"{statistics.mean(times):.2f}",
                "Median Time (ms)": f"{statistics.median(times):.2f}",
                "Avg Bytes Removed": f"{statistics.mean(bytes_removed):,.0f}",
                "Avg Rows Removed": f"{statistics.mean(rows_removed):.1f}",
                "Total Bytes Removed": f"{sum(bytes_removed):,}",
                "Total Rows Removed": f"{sum(rows_removed):.0f}",
            }

        # Build table
        metrics = ["Tests", "Avg Time (ms)", "Median Time (ms)", "Avg Bytes Removed",
                   "Avg Rows Removed", "Total Bytes Removed", "Total Rows Removed"]

        for metric in metrics:
            row = [metric]
            for tool in ["Reference Cleaner", "Toolkit"]:
                results = by_tool.get(tool, [])
                if not results:
                    row.append("-")
                    continue

                times = [r["time_ms"] for r in results]
                bytes_removed = [r["bytes_removed"] for r in results]
                rows_removed = [r["rows_removed"] for r in results]

                if metric == "Tests":
                    row.append(str(len(results)))
                elif metric == "Avg Time (ms)":
                    row.append(f"{statistics.mean(times):.2f}")
                elif metric == "Median Time (ms)":
                    row.append(f"{statistics.median(times):.2f}")
                elif metric == "Avg Bytes Removed":
                    row.append(f"{statistics.mean(bytes_removed):,.0f}")
                elif metric == "Avg Rows Removed":
                    row.append(f"{statistics.mean(rows_removed):.1f}")
                elif metric == "Total Bytes Removed":
                    row.append(f"{sum(bytes_removed):,}")
                elif metric == "Total Rows Removed":
                    row.append(f"{sum(rows_removed):.0f}")

            rows.append(row)

        table = ax.table(cellText=rows, cellLoc='center', loc='center',
                        colWidths=[0.3, 0.35, 0.35])
        table.auto_set_font_size(False)
        table.set_fontsize(10)
        table.scale(1, 2)

        # Style header row
        for i in range(3):
            table[(0, i)].set_facecolor('#cccccc')
            table[(0, i)].set_text_props(weight='bold')

        # Alternate row colors
        for i in range(1, len(rows)):
            for j in range(3):
                if i % 2 == 0:
                    table[(i, j)].set_facecolor('#f0f0f0')

        plt.title('Summary Statistics', fontsize=FONT_SIZE + 2, pad=20)
        plt.tight_layout()
        output_path = self.output_dir / "summary_statistics.png"
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()

    def generate_all(self):
        """Generate all visualizations"""
        print("Generating visualizations...")
        self.performance_comparison()
        self.effectiveness_comparison()
        self.dataset_breakdown()
        self.summary_statistics()
        print("\nAll visualizations complete!")

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Visualize real dataset benchmark results")
    parser.add_argument("--results", help="Path to benchmark results JSON file")
    parser.add_argument("--output", help="Output directory for images")

    args = parser.parse_args()

    visualizer = RealBenchmarkVisualizer(
        results_file=Path(args.results) if args.results else None,
        output_dir=Path(args.output) if args.output else None
    )
    visualizer.generate_all()
