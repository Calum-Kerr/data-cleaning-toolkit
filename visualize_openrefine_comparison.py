#!/usr/bin/env python3
"""
Generate dissertation-quality visualizations from OpenRefine comparison benchmarks
Creates publication-ready charts and statistical summaries
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
    'Toolkit': '#0000ee',
    'OpenRefine': '#ff6b6b',
    'OpenRefine (simulated)': '#ff6b6b'
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

class BenchmarkVisualizer:
    def __init__(self, results_file: Path = None, output_dir: Path = None):
        self.results_file = results_file or (
            Path(__file__).parent / "benchmark_results" / "openrefine_comparison_results.json"
        )
        self.output_dir = output_dir or (Path(__file__).parent / "benchmark_results")
        self.output_dir.mkdir(exist_ok=True)

        self.results = []
        self.load_results()

    def load_results(self):
        """Load benchmark results from JSON"""
        if not self.results_file.exists():
            print(f"Error: Results file not found at {self.results_file}")
            return

        with open(self.results_file) as f:
            data = json.load(f)
            self.results = data.get("results", [])

        print(f"Loaded {len(self.results)} benchmark results")

    def get_tool_results(self, tool_name: str) -> List[Dict]:
        """Filter results by tool"""
        return [r for r in self.results if r.get("tool") == tool_name and r.get("status") == "success"]

    def performance_comparison(self):
        """Create performance (execution time) comparison chart"""
        fig, ax = plt.subplots(figsize=(12, 6))

        tools = list(set(r.get("tool") for r in self.results if r.get("status") == "success"))
        times_by_tool = {tool: [] for tool in tools}

        for result in self.results:
            if result.get("status") == "success":
                tool = result.get("tool")
                times_by_tool[tool].append(result.get("time_ms", 0))

        # Create box plot
        data = [times_by_tool[tool] for tool in sorted(tools)]
        bp = ax.boxplot(data, labels=sorted(tools), patch_artist=True)

        # Color the boxes
        for patch, tool in zip(bp['boxes'], sorted(tools)):
            patch.set_facecolor(COLORS.get(tool, '#cccccc'))
            patch.set_alpha(0.7)

        ax.set_ylabel('Execution Time (milliseconds)', fontsize=FONT_SIZE+1)
        ax.set_title('Performance Comparison: Toolkit vs OpenRefine', fontsize=FONT_SIZE+2, fontweight='bold')
        ax.grid(True, alpha=0.3, axis='y')

        # Add statistics text
        stats_text = "Execution Time Statistics:\n"
        for tool in sorted(tools):
            times = times_by_tool[tool]
            stats_text += f"\n{tool}:\n"
            stats_text += f"  Mean: {statistics.mean(times):.2f}ms\n"
            stats_text += f"  Median: {statistics.median(times):.2f}ms\n"
            stats_text += f"  StdDev: {statistics.stdev(times):.2f}ms" if len(times) > 1 else ""

        ax.text(0.98, 0.97, stats_text, transform=ax.transAxes,
                verticalalignment='top', horizontalalignment='right',
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5),
                fontsize=FONT_SIZE-1, family='monospace')

        plt.tight_layout()
        filepath = self.output_dir / "performance_comparison.png"
        plt.savefig(filepath, dpi=300, bbox_inches='tight')
        print(f"Saved: {filepath}")
        plt.close()

    def effectiveness_comparison(self):
        """Create effectiveness (bytes/rows removed) comparison"""
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

        tools = list(set(r.get("tool") for r in self.results if r.get("status") == "success"))

        # Bytes removed comparison
        bytes_by_tool = {tool: [] for tool in tools}
        rows_by_tool = {tool: [] for tool in tools}

        for result in self.results:
            if result.get("status") == "success":
                tool = result.get("tool")
                bytes_by_tool[tool].append(result.get("bytes_removed", 0))
                rows_by_tool[tool].append(result.get("rows_removed", 0))

        # Bytes removed
        x = np.arange(len(tools))
        width = 0.35
        means_bytes = [statistics.mean(bytes_by_tool[tool]) for tool in sorted(tools)]

        bars1 = ax1.bar(x, means_bytes, width, label='Average Bytes Removed')
        ax1.set_ylabel('Bytes Removed (per dataset)', fontsize=FONT_SIZE+1)
        ax1.set_title('Data Reduction: Average Bytes Removed', fontsize=FONT_SIZE+2, fontweight='bold')
        ax1.set_xticks(x)
        ax1.set_xticklabels(sorted(tools))
        ax1.grid(True, alpha=0.3, axis='y')

        # Add value labels on bars
        for bar in bars1:
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height,
                    f'{int(height)}', ha='center', va='bottom', fontsize=FONT_SIZE-1)

        # Rows removed
        means_rows = [statistics.mean(rows_by_tool[tool]) for tool in sorted(tools)]
        bars2 = ax2.bar(x, means_rows, width, label='Average Rows Removed')
        ax2.set_ylabel('Rows Removed (per dataset)', fontsize=FONT_SIZE+1)
        ax2.set_title('Deduplication: Average Rows Removed', fontsize=FONT_SIZE+2, fontweight='bold')
        ax2.set_xticks(x)
        ax2.set_xticklabels(sorted(tools))
        ax2.grid(True, alpha=0.3, axis='y')

        # Add value labels
        for bar in bars2:
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.1f}', ha='center', va='bottom', fontsize=FONT_SIZE-1)

        plt.tight_layout()
        filepath = self.output_dir / "effectiveness_comparison.png"
        plt.savefig(filepath, dpi=300, bbox_inches='tight')
        print(f"Saved: {filepath}")
        plt.close()

    def dataset_breakdown(self):
        """Create dataset-by-dataset comparison"""
        # Group by dataset
        datasets = list(set(r.get("dataset") for r in self.results if r.get("status") == "success"))

        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10))

        # Performance by dataset
        for tool in set(r.get("tool") for r in self.results if r.get("status") == "success"):
            times = []
            for dataset in sorted(datasets):
                dataset_results = [r for r in self.results
                                 if r.get("dataset") == dataset and r.get("tool") == tool
                                 and r.get("status") == "success"]
                if dataset_results:
                    times.append(statistics.mean([r.get("time_ms", 0) for r in dataset_results]))
                else:
                    times.append(0)

            ax1.plot(sorted(datasets), times, marker='o', label=tool, linewidth=2, markersize=6)

        ax1.set_ylabel('Execution Time (ms)', fontsize=FONT_SIZE+1)
        ax1.set_title('Performance by Dataset', fontsize=FONT_SIZE+2, fontweight='bold')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        plt.setp(ax1.xaxis.get_majorticklabels(), rotation=45, ha='right')

        # Bytes removed by dataset
        for tool in set(r.get("tool") for r in self.results if r.get("status") == "success"):
            bytes_removed = []
            for dataset in sorted(datasets):
                dataset_results = [r for r in self.results
                                 if r.get("dataset") == dataset and r.get("tool") == tool
                                 and r.get("status") == "success"]
                if dataset_results:
                    bytes_removed.append(statistics.mean([r.get("bytes_removed", 0) for r in dataset_results]))
                else:
                    bytes_removed.append(0)

            ax2.plot(sorted(datasets), bytes_removed, marker='s', label=tool, linewidth=2, markersize=6)

        ax2.set_xlabel('Dataset', fontsize=FONT_SIZE+1)
        ax2.set_ylabel('Bytes Removed', fontsize=FONT_SIZE+1)
        ax2.set_title('Data Reduction by Dataset', fontsize=FONT_SIZE+2, fontweight='bold')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        plt.setp(ax2.xaxis.get_majorticklabels(), rotation=45, ha='right')

        plt.tight_layout()
        filepath = self.output_dir / "dataset_breakdown.png"
        plt.savefig(filepath, dpi=300, bbox_inches='tight')
        print(f"Saved: {filepath}")
        plt.close()

    def summary_statistics_table(self):
        """Generate summary statistics table as image"""
        fig, ax = plt.subplots(figsize=(12, 6))
        ax.axis('tight')
        ax.axis('off')

        tools = list(set(r.get("tool") for r in self.results if r.get("status") == "success"))

        table_data = [["Metric", *sorted(tools)]]

        # Add rows
        for tool in sorted(tools):
            tool_results = self.get_tool_results(tool)
            times = [r.get("time_ms", 0) for r in tool_results]
            bytes_removed = [r.get("bytes_removed", 0) for r in tool_results]
            rows_removed = [r.get("rows_removed", 0) for r in tool_results]

        metrics = [
            ("Count", lambda tool: len(self.get_tool_results(tool))),
            ("Mean Time (ms)", lambda tool: f"{statistics.mean([r.get('time_ms', 0) for r in self.get_tool_results(tool)]):.2f}"),
            ("Median Time (ms)", lambda tool: f"{statistics.median([r.get('time_ms', 0) for r in self.get_tool_results(tool)]):.2f}"),
            ("StdDev Time (ms)", lambda tool: f"{statistics.stdev([r.get('time_ms', 0) for r in self.get_tool_results(tool)]):.2f}" if len(self.get_tool_results(tool)) > 1 else "N/A"),
            ("Avg Bytes Removed", lambda tool: f"{statistics.mean([r.get('bytes_removed', 0) for r in self.get_tool_results(tool)]):.0f}"),
            ("Avg Rows Removed", lambda tool: f"{statistics.mean([r.get('rows_removed', 0) for r in self.get_tool_results(tool)]):.1f}"),
            ("Total Bytes Removed", lambda tool: f"{sum([r.get('bytes_removed', 0) for r in self.get_tool_results(tool)]):,}"),
            ("Total Rows Removed", lambda tool: f"{sum([r.get('rows_removed', 0) for r in self.get_tool_results(tool)])}"),
        ]

        for metric_name, metric_fn in metrics:
            row = [metric_name]
            for tool in sorted(tools):
                row.append(str(metric_fn(tool)))
            table_data.append(row)

        table = ax.table(cellText=table_data, cellLoc='center', loc='center',
                        colWidths=[0.3] + [0.35] * len(tools))

        table.auto_set_font_size(False)
        table.set_fontsize(FONT_SIZE)
        table.scale(1, 2)

        # Style header row
        for i in range(len(table_data[0])):
            table[(0, i)].set_facecolor('#0000ee')
            table[(0, i)].set_text_props(weight='bold', color='white')

        plt.title('Comparison Summary Statistics', fontsize=FONT_SIZE+2, fontweight='bold', pad=20)

        plt.tight_layout()
        filepath = self.output_dir / "summary_statistics.png"
        plt.savefig(filepath, dpi=300, bbox_inches='tight')
        print(f"Saved: {filepath}")
        plt.close()

    def generate_all(self):
        """Generate all visualizations"""
        if not self.results:
            print("No results to visualize. Run benchmark first.")
            return

        print("\nGenerating dissertation-quality visualizations...")
        print("-" * 60)

        self.performance_comparison()
        self.effectiveness_comparison()
        self.dataset_breakdown()
        self.summary_statistics_table()

        print("-" * 60)
        print(f"All visualizations saved to: {self.output_dir}")
        print("\nGenerated files:")
        print("  - performance_comparison.png")
        print("  - effectiveness_comparison.png")
        print("  - dataset_breakdown.png")
        print("  - summary_statistics.png")

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Visualize OpenRefine comparison benchmarks")
    parser.add_argument("--results", help="Path to results JSON file")
    parser.add_argument("--output", help="Output directory for visualizations")

    args = parser.parse_args()

    visualizer = BenchmarkVisualizer(
        results_file=Path(args.results) if args.results else None,
        output_dir=Path(args.output) if args.output else None
    )
    visualizer.generate_all()
