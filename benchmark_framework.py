#!/usr/bin/env python3

import csv
import json
import time
import statistics
import os
from pathlib import Path
from typing import Dict, List, Tuple

class BenchmarkResult:
    def __init__(self, tool_name: str, dataset_name: str, task_name: str):
        self.tool_name = tool_name
        self.dataset_name = dataset_name
        self.task_name = task_name
        self.execution_times = []
        self.audit_log_lines = 0
        self.rows_processed = 0
        self.accuracy = 0.0
        self.errors_detected = 0

    def add_execution_time(self, time_ms: float):
        self.execution_times.append(time_ms)

    def get_statistics(self) -> Dict:
        if not self.execution_times:
            return {"error": "No execution times recorded"}

        return {
            "tool": self.tool_name,
            "dataset": self.dataset_name,
            "task": self.task_name,
            "runs": len(self.execution_times),
            "mean_ms": round(statistics.mean(self.execution_times), 2),
            "median_ms": round(statistics.median(self.execution_times), 2),
            "stdev_ms": round(statistics.stdev(self.execution_times), 2) if len(self.execution_times) > 1 else 0,
            "min_ms": round(min(self.execution_times), 2),
            "max_ms": round(max(self.execution_times), 2),
            "rows_processed": self.rows_processed,
            "accuracy": round(self.accuracy, 4),
            "errors_detected": self.errors_detected,
            "audit_log_lines": self.audit_log_lines
        }

class BenchmarkFramework:
    def __init__(self, datasets_path: str):
        self.datasets_path = Path(datasets_path)
        self.results = []
        self.datasets = {
            "Airbnb": {"tasks": ["duplicates", "outliers"], "rows": 23203},
            "Credit": {"tasks": ["outliers"], "rows": 105001},
            "Titanic": {"tasks": ["missing_values"], "rows": 625}
        }

    def load_csv(self, filepath: str) -> List[List[str]]:
        with open(filepath, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            return list(reader)

    def count_errors_in_indicator(self, filepath: str) -> int:
        data = self.load_csv(filepath)
        error_count = 0
        for row in data[1:]:
            if row and row[0] != '0':
                error_count += 1
        return error_count

    def get_dataset_info(self, dataset_name: str, task_name: str) -> Dict:
        dataset_path = self.datasets_path / dataset_name / task_name
        dirty_file = dataset_path / "dirty_train.csv"
        clean_file = dataset_path / "clean_train.csv"
        indicator_file = dataset_path / f"indicator_clean_train.csv"

        if not dirty_file.exists():
            return None

        dirty_data = self.load_csv(str(dirty_file))
        total_rows = len(dirty_data) - 1
        total_errors = self.count_errors_in_indicator(str(indicator_file)) if indicator_file.exists() else 0

        return {
            "dataset": dataset_name,
            "task": task_name,
            "dirty_file": str(dirty_file),
            "clean_file": str(clean_file),
            "indicator_file": str(indicator_file),
            "total_rows": total_rows,
            "total_errors": total_errors,
            "error_percentage": round((total_errors / total_rows * 100), 2) if total_rows > 0 else 0
        }

    def print_experiment_plan(self):
        print("\n=== BENCHMARK EXPERIMENT PLAN ===\n")
        for dataset_name, info in self.datasets.items():
            print(f"\n{dataset_name} Dataset ({info['rows']} rows):")
            for task in info['tasks']:
                dataset_info = self.get_dataset_info(dataset_name, task)
                if dataset_info:
                    print(f"  Task: {task}")
                    print(f"    Total rows: {dataset_info['total_rows']}")
                    print(f"    Error rows: {dataset_info['total_errors']} ({dataset_info['error_percentage']}%)")
                    print(f"    Input: {dataset_info['dirty_file']}")

    def export_results_json(self, filepath: str):
        export_data = {
            "benchmark_run": time.strftime("%Y-%m-%d %H:%M:%S"),
            "results": [r.get_statistics() for r in self.results]
        }
        with open(filepath, 'w') as f:
            json.dump(export_data, f, indent=2)
        print(f"\nResults exported to {filepath}")

    def create_comparative_table(self) -> str:
        if not self.results:
            return "No results to display"

        table = "\n=== COMPARATIVE BENCHMARK TABLE ===\n"
        table += f"{'Tool':<15} {'Dataset':<12} {'Task':<15} {'Mean (ms)':<12} {'Median (ms)':<12} {'StDev':<10} {'Accuracy':<10}\n"
        table += "-" * 85 + "\n"

        for result in self.results:
            stats = result.get_statistics()
            table += f"{stats['tool']:<15} {stats['dataset']:<12} {stats['task']:<15} {stats['mean_ms']:<12.2f} {stats['median_ms']:<12.2f} {stats['stdev_ms']:<10.2f} {stats['accuracy']:<10.4f}\n"

        return table

if __name__ == "__main__":
    framework = BenchmarkFramework("/Users/ck/Desktop/controlled-refactor/data-cleaning-toolkit/datasets")
    framework.print_experiment_plan()
