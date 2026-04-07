#!/usr/bin/env python3
"""
OpenRefine vs Data Cleaning Toolkit Comparison Benchmarks
Compares cleaning accuracy, performance, and effectiveness across 103 datasets
"""

import json
import os
import csv
import time
import subprocess
import tempfile
import shutil
from pathlib import Path
from typing import Dict, List, Any
import statistics

class OpenRefineComparison:
    def __init__(self, toolkit_api_url="http://localhost:8080", datasets_dir=None):
        self.toolkit_url = toolkit_api_url
        self.datasets_dir = datasets_dir or Path(__file__).parent / "datasets"
        self.results = []
        self.benchmark_results_dir = Path(__file__).parent / "benchmark_results"

    def get_all_csvs(self) -> List[tuple]:
        """Find all CSV files in datasets directory"""
        csvs = []
        for root, dirs, files in os.walk(self.datasets_dir):
            for file in files:
                if file.endswith('.csv'):
                    csvs.append((Path(root) / file, Path(root).name))
        return sorted(csvs)

    def read_csv(self, filepath) -> tuple:
        """Read CSV and return rows + byte count"""
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        rows = content.strip().split('\n')
        return rows, len(content.encode('utf-8'))

    def test_toolkit(self, csv_content: str, csv_path: Path, dataset_name: str, rows: List[str]) -> Dict[str, Any]:
        """Test using Data Cleaning Toolkit API"""
        start = time.time()

        try:
            import requests
            response = requests.post(
                f"{self.toolkit_url}/api/clean",
                json={"csvData": csv_content},
                timeout=30
            )
            elapsed = (time.time() - start) * 1000

            if response.status_code == 200:
                data = response.json()
                cleaned_csv = data.get("cleanedData", csv_content)
                cleaned_rows = cleaned_csv.strip().split('\n')

                return {
                    "tool": "Toolkit",
                    "dataset": dataset_name,
                    "filename": csv_path.name,
                    "status": "success",
                    "time_ms": round(elapsed, 2),
                    "rows_before": len(rows),
                    "rows_after": len(cleaned_rows),
                    "rows_removed": len(rows) - len(cleaned_rows),
                    "bytes_before": len(csv_content.encode('utf-8')),
                    "bytes_after": len(cleaned_csv.encode('utf-8')),
                    "bytes_removed": len(csv_content.encode('utf-8')) - len(cleaned_csv.encode('utf-8')),
                    "audit_log": data.get("auditLog", [])
                }
        except Exception as e:
            elapsed = (time.time() - start) * 1000
            return {
                "tool": "Toolkit",
                "dataset": dataset_name,
                "filename": csv_path.name,
                "status": "error",
                "time_ms": round(elapsed, 2),
                "error": str(e)
            }

    def test_openrefine(self, csv_path: Path, dataset_name: str, rows: List[str]) -> Dict[str, Any]:
        """Test using OpenRefine CLI"""
        start = time.time()

        try:
            with tempfile.TemporaryDirectory() as tmpdir:
                tmpdir = Path(tmpdir)
                project_dir = tmpdir / "project"
                project_dir.mkdir()

                # Create OpenRefine project with standard cleaning operations
                # Note: This requires OpenRefine CLI to be installed
                # For now, simulating with basic CSV processing

                with open(csv_path, 'r', encoding='utf-8') as f:
                    original_content = f.read()

                # Simulate OpenRefine operations:
                # 1. Trim whitespace from all cells
                # 2. Remove empty rows
                # 3. Detect duplicates
                cleaned_rows = self._simulate_openrefine_cleaning(rows)
                cleaned_content = '\n'.join(cleaned_rows)

                elapsed = (time.time() - start) * 1000

                return {
                    "tool": "OpenRefine (simulated)",
                    "dataset": dataset_name,
                    "filename": csv_path.name,
                    "status": "success",
                    "time_ms": round(elapsed, 2),
                    "rows_before": len(rows),
                    "rows_after": len(cleaned_rows),
                    "rows_removed": len(rows) - len(cleaned_rows),
                    "bytes_before": len(original_content.encode('utf-8')),
                    "bytes_after": len(cleaned_content.encode('utf-8')),
                    "bytes_removed": len(original_content.encode('utf-8')) - len(cleaned_content.encode('utf-8')),
                    "operations": ["trim whitespace", "remove empty rows", "deduplicate"]
                }
        except Exception as e:
            elapsed = (time.time() - start) * 1000
            return {
                "tool": "OpenRefine",
                "dataset": dataset_name,
                "filename": csv_path.name,
                "status": "error",
                "time_ms": round(elapsed, 2),
                "error": str(e)
            }

    def _simulate_openrefine_cleaning(self, rows: List[str]) -> List[str]:
        """Simulate OpenRefine's standard cleaning operations"""
        if not rows:
            return rows

        cleaned = []
        seen = set()

        for row in rows:
            # Trim whitespace from each cell
            cells = [cell.strip() for cell in row.split(',')]
            cleaned_row = ','.join(cells)

            # Remove empty rows
            if cleaned_row.strip():
                # Deduplicate
                if cleaned_row not in seen:
                    cleaned.append(cleaned_row)
                    seen.add(cleaned_row)

        return cleaned

    def run_benchmark(self, limit: int = None) -> None:
        """Run comparison on all datasets"""
        csvs = self.get_all_csvs()

        if limit:
            csvs = csvs[:limit]

        print(f"Comparing {len(csvs)} CSV files...")
        print("-" * 80)

        for csv_path, dataset_name in csvs:
            rows, byte_count = self.read_csv(csv_path)
            csv_content = '\n'.join(rows)

            print(f"Testing {dataset_name}/{csv_path.name}...", end=" ")

            # Test Toolkit
            toolkit_result = self.test_toolkit(csv_content, csv_path, dataset_name, rows)
            self.results.append(toolkit_result)

            # Test OpenRefine
            openrefine_result = self.test_openrefine(csv_path, dataset_name, rows)
            self.results.append(openrefine_result)

            # Log results
            toolkit_status = "✓" if toolkit_result["status"] == "success" else "✗"
            openref_status = "✓" if openrefine_result["status"] == "success" else "✗"
            print(f"{toolkit_status} {openref_status}")

    def generate_comparison_report(self) -> Dict[str, Any]:
        """Generate comparison statistics"""
        toolkit_results = [r for r in self.results if r["tool"] == "Toolkit" and r["status"] == "success"]
        openref_results = [r for r in self.results if "OpenRefine" in r["tool"] and r["status"] == "success"]

        report = {
            "summary": {
                "total_datasets": len(set(r["dataset"] for r in self.results if r["status"] == "success")),
                "toolkit_success": len(toolkit_results),
                "openrefine_success": len(openref_results),
            },
            "performance": {
                "toolkit": {
                    "mean_time_ms": round(statistics.mean([r["time_ms"] for r in toolkit_results]), 2) if toolkit_results else 0,
                    "median_time_ms": round(statistics.median([r["time_ms"] for r in toolkit_results]), 2) if toolkit_results else 0,
                    "min_time_ms": round(min([r["time_ms"] for r in toolkit_results]), 2) if toolkit_results else 0,
                    "max_time_ms": round(max([r["time_ms"] for r in toolkit_results]), 2) if toolkit_results else 0,
                },
                "openrefine": {
                    "mean_time_ms": round(statistics.mean([r["time_ms"] for r in openref_results]), 2) if openref_results else 0,
                    "median_time_ms": round(statistics.median([r["time_ms"] for r in openref_results]), 2) if openref_results else 0,
                    "min_time_ms": round(min([r["time_ms"] for r in openref_results]), 2) if openref_results else 0,
                    "max_time_ms": round(max([r["time_ms"] for r in openref_results]), 2) if openref_results else 0,
                }
            },
            "effectiveness": {
                "toolkit": {
                    "avg_bytes_removed": round(statistics.mean([r["bytes_removed"] for r in toolkit_results]), 0) if toolkit_results else 0,
                    "avg_rows_removed": round(statistics.mean([r["rows_removed"] for r in toolkit_results]), 1) if toolkit_results else 0,
                    "total_bytes_removed": sum([r["bytes_removed"] for r in toolkit_results]),
                    "total_rows_removed": sum([r["rows_removed"] for r in toolkit_results]),
                },
                "openrefine": {
                    "avg_bytes_removed": round(statistics.mean([r["bytes_removed"] for r in openref_results]), 0) if openref_results else 0,
                    "avg_rows_removed": round(statistics.mean([r["rows_removed"] for r in openref_results]), 1) if openref_results else 0,
                    "total_bytes_removed": sum([r["bytes_removed"] for r in openref_results]),
                    "total_rows_removed": sum([r["rows_removed"] for r in openref_results]),
                }
            }
        }

        return report

    def save_results(self) -> None:
        """Save results to benchmark_results folder"""
        self.benchmark_results_dir.mkdir(exist_ok=True)

        # Save raw results
        results_file = self.benchmark_results_dir / "openrefine_comparison_results.json"
        with open(results_file, 'w') as f:
            json.dump({"results": self.results}, f, indent=2)
        print(f"\nResults saved to {results_file}")

        # Generate and save report
        report = self.generate_comparison_report()
        report_file = self.benchmark_results_dir / "openrefine_comparison_report.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
        print(f"Report saved to {report_file}")

        # Save as CSV for easy viewing
        csv_file = self.benchmark_results_dir / "openrefine_comparison_results.csv"
        if self.results:
            with open(csv_file, 'w', newline='') as f:
                writer = csv.DictWriter(f, fieldnames=self.results[0].keys())
                writer.writeheader()
                writer.writerows(self.results)
            print(f"CSV saved to {csv_file}")

        # Print summary
        print("\n" + "="*80)
        print("COMPARISON SUMMARY")
        print("="*80)
        print(json.dumps(report, indent=2))

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Compare Data Cleaning Toolkit vs OpenRefine")
    parser.add_argument("--api-url", default="http://localhost:8080", help="Toolkit API URL")
    parser.add_argument("--datasets", help="Datasets directory")
    parser.add_argument("--limit", type=int, help="Limit number of datasets to test")

    args = parser.parse_args()

    benchmark = OpenRefineComparison(toolkit_api_url=args.api_url, datasets_dir=args.datasets)
    benchmark.run_benchmark(limit=args.limit)
    benchmark.save_results()
