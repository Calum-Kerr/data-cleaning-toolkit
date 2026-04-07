#!/usr/bin/env python3
"""
Real Dataset Benchmark: Data Cleaning Toolkit vs Reference Cleaner
Tests on actual datasets from /datasets/ folder with real dirty data
"""

import json
import csv
import os
import time
from pathlib import Path
from typing import Dict, List, Any, Tuple
import statistics

class CSVCleaner:
    """Reference CSV cleaner that implements standard operations"""

    @staticmethod
    def clean(csv_content: str) -> Tuple[str, Dict[str, Any]]:
        """
        Clean CSV data with standard operations:
        1. Trim whitespace from all cells
        2. Remove empty rows
        3. Detect and mark duplicates
        4. Handle null values
        5. Collapse multiple spaces
        """
        lines = csv_content.strip().split('\n')
        if not lines:
            return csv_content, {"operations": []}

        cleaned_lines = []
        seen_rows = set()
        duplicates = 0
        empty_rows = 0
        whitespace_rows = 0
        nulls_found = 0
        spaces_collapsed = 0

        for i, line in enumerate(lines):
            # Parse CSV line (handle quoted fields)
            cells = CSVCleaner.parse_csv_line(line)

            # Trim each cell and track whitespace changes
            trimmed_cells = []
            for cell in cells:
                original = cell
                # Trim whitespace
                trimmed = cell.strip()
                # Collapse multiple spaces
                trimmed = ' '.join(trimmed.split())

                if trimmed != original:
                    if original != original.strip():
                        whitespace_rows += 1
                    if '  ' in original:
                        spaces_collapsed += 1

                # Track nulls
                if trimmed.lower() in ['null', 'none', 'na', 'n/a', '']:
                    if trimmed.lower() not in ['']:  # Don't count empty as null
                        nulls_found += 1

                trimmed_cells.append(trimmed)

            # Reconstruct line
            cleaned_line = CSVCleaner.format_csv_line(trimmed_cells)

            # Skip empty rows
            if not cleaned_line.strip() or all(c == '' for c in trimmed_cells):
                empty_rows += 1
                continue

            # Detect duplicates
            if cleaned_line in seen_rows:
                duplicates += 1
                continue

            seen_rows.add(cleaned_line)
            cleaned_lines.append(cleaned_line)

        cleaned_csv = '\n'.join(cleaned_lines)

        audit = {
            "operations": [
                {"name": "trim whitespace", "affected": whitespace_rows},
                {"name": "collapse multiple spaces", "affected": spaces_collapsed},
                {"name": "remove empty rows", "affected": empty_rows},
                {"name": "remove duplicates", "affected": duplicates},
                {"name": "null values detected", "affected": nulls_found},
            ],
            "bytes_removed": len(csv_content.encode('utf-8')) - len(cleaned_csv.encode('utf-8')),
            "rows_removed": len(lines) - len(cleaned_lines),
            "duplicates_found": duplicates,
            "nulls_detected": nulls_found,
        }

        return cleaned_csv, audit

    @staticmethod
    def parse_csv_line(line: str) -> List[str]:
        """Parse CSV line handling quoted fields"""
        cells = []
        current = ''
        in_quotes = False

        for char in line:
            if char == '"':
                in_quotes = not in_quotes
            elif char == ',' and not in_quotes:
                cells.append(current)
                current = ''
            else:
                current += char

        cells.append(current)
        return cells

    @staticmethod
    def format_csv_line(cells: List[str]) -> str:
        """Format cells back to CSV line"""
        # Quote cells that contain commas or quotes
        quoted = []
        for cell in cells:
            if ',' in cell or '"' in cell:
                cell = '"' + cell.replace('"', '""') + '"'
            quoted.append(cell)
        return ','.join(quoted)


class RealDatasetBenchmark:
    def __init__(self, datasets_dir: Path = None, toolkit_api_url: str = "http://localhost:8080"):
        self.datasets_dir = datasets_dir or (Path(__file__).parent / "datasets")
        self.toolkit_url = toolkit_api_url
        self.results = []
        self.benchmark_results_dir = Path(__file__).parent / "benchmark_results"

    def find_dirty_csvs(self) -> List[Tuple[Path, str]]:
        """Find all dirty/test CSV files"""
        dirty_files = []

        for root, dirs, files in os.walk(self.datasets_dir):
            for file in files:
                if file.endswith('.csv'):
                    filepath = Path(root) / file
                    # Include files with "dirty", "test", or just any CSV
                    if any(x in file.lower() for x in ['dirty', 'test', '.csv']):
                        dataset_name = Path(root).parent.name
                        dirty_files.append((filepath, dataset_name))

        return sorted(dirty_files)  # Test all datasets

    def test_reference_cleaner(self, csv_path: Path, dataset_name: str) -> Dict[str, Any]:
        """Test using reference CSV cleaner"""
        try:
            with open(csv_path, 'r', encoding='utf-8') as f:
                original_content = f.read()

            start = time.time()
            cleaned_content, audit = CSVCleaner.clean(original_content)
            elapsed = (time.time() - start) * 1000

            original_rows = len(original_content.strip().split('\n'))
            cleaned_rows = len(cleaned_content.strip().split('\n')) if cleaned_content.strip() else 0

            return {
                "tool": "Reference Cleaner",
                "dataset": dataset_name,
                "filename": csv_path.name,
                "status": "success",
                "time_ms": round(elapsed, 2),
                "rows_before": original_rows,
                "rows_after": cleaned_rows,
                "rows_removed": original_rows - cleaned_rows,
                "bytes_before": len(original_content.encode('utf-8')),
                "bytes_after": len(cleaned_content.encode('utf-8')),
                "bytes_removed": audit["bytes_removed"],
                "duplicates_found": audit["duplicates_found"],
                "nulls_detected": audit["nulls_detected"],
                "audit_log": audit["operations"],
            }
        except Exception as e:
            return {
                "tool": "Reference Cleaner",
                "dataset": dataset_name,
                "filename": csv_path.name,
                "status": "error",
                "error": str(e),
            }

    def test_toolkit(self, csv_content: str, csv_path: Path, dataset_name: str) -> Dict[str, Any]:
        """Test using Toolkit API"""
        try:
            import requests

            start = time.time()
            response = requests.post(
                f"{self.toolkit_url}/api/clean",
                json={"csvData": csv_content},
                timeout=10
            )
            elapsed = (time.time() - start) * 1000

            if response.status_code == 200:
                data = response.json()
                cleaned_csv = data.get("cleanedData", csv_content)
                original_rows = len(csv_content.strip().split('\n'))
                cleaned_rows = len(cleaned_csv.strip().split('\n')) if cleaned_csv.strip() else 0

                return {
                    "tool": "Toolkit",
                    "dataset": dataset_name,
                    "filename": csv_path.name,
                    "status": "success",
                    "time_ms": round(elapsed, 2),
                    "rows_before": original_rows,
                    "rows_after": cleaned_rows,
                    "rows_removed": original_rows - cleaned_rows,
                    "bytes_before": len(csv_content.encode('utf-8')),
                    "bytes_after": len(cleaned_csv.encode('utf-8')),
                    "bytes_removed": len(csv_content.encode('utf-8')) - len(cleaned_csv.encode('utf-8')),
                    "audit_log": data.get("auditLog", []),
                }
            else:
                return {
                    "tool": "Toolkit",
                    "dataset": dataset_name,
                    "filename": csv_path.name,
                    "status": "error",
                    "error": f"API returned {response.status_code}",
                }
        except requests.exceptions.ConnectionError:
            return {
                "tool": "Toolkit",
                "dataset": dataset_name,
                "filename": csv_path.name,
                "status": "skipped",
                "error": "API not reachable",
            }
        except Exception as e:
            return {
                "tool": "Toolkit",
                "dataset": dataset_name,
                "filename": csv_path.name,
                "status": "error",
                "error": str(e),
            }

    def run_benchmark(self) -> None:
        """Run comparison on real datasets"""
        csvs = self.find_dirty_csvs()

        print(f"Testing {len(csvs)} CSV files...")
        print("-" * 80)

        for csv_path, dataset_name in csvs:
            try:
                with open(csv_path, 'r', encoding='utf-8') as f:
                    csv_content = f.read()
            except Exception as e:
                print(f"Error reading {csv_path.name}: {e}")
                continue

            print(f"Testing {dataset_name}/{csv_path.name}...", end=" ")

            # Test reference cleaner
            ref_result = self.test_reference_cleaner(csv_path, dataset_name)
            self.results.append(ref_result)

            # Test Toolkit
            toolkit_result = self.test_toolkit(csv_content, csv_path, dataset_name)
            self.results.append(toolkit_result)

            # Log results
            ref_status = "✓" if ref_result["status"] == "success" else "✗"
            tk_status = "✓" if toolkit_result["status"] == "success" else "✗"
            print(f"{ref_status} {tk_status}")

    def save_results(self) -> None:
        """Save results to benchmark_results folder"""
        self.benchmark_results_dir.mkdir(exist_ok=True)

        # Save raw results
        results_file = self.benchmark_results_dir / "real_dataset_benchmark_results.json"
        with open(results_file, 'w') as f:
            json.dump({"results": self.results}, f, indent=2)
        print(f"\nResults saved to {results_file}")

        # Save as CSV
        csv_file = self.benchmark_results_dir / "real_dataset_benchmark_results.csv"
        if self.results:
            all_fields = set()
            for result in self.results:
                all_fields.update(result.keys())
            fieldnames = sorted(list(all_fields))

            with open(csv_file, 'w', newline='') as f:
                writer = csv.DictWriter(f, fieldnames=fieldnames, restval='')
                writer.writeheader()
                writer.writerows(self.results)
            print(f"CSV saved to {csv_file}")

        # Print summary
        successful = [r for r in self.results if r.get("status") == "success"]
        if successful:
            print("\n" + "=" * 80)
            print("BENCHMARK SUMMARY")
            print("=" * 80)

            by_tool = {}
            for result in successful:
                tool = result.get("tool")
                if tool not in by_tool:
                    by_tool[tool] = []
                by_tool[tool].append(result)

            for tool, results in by_tool.items():
                times = [r["time_ms"] for r in results]
                bytes_removed = [r["bytes_removed"] for r in results]
                rows_removed = [r["rows_removed"] for r in results]

                print(f"\n{tool}:")
                print(f"  Tests: {len(results)}")
                print(f"  Avg Time: {statistics.mean(times):.2f}ms")
                print(f"  Avg Bytes Removed: {statistics.mean(bytes_removed):.0f}")
                print(f"  Avg Rows Removed: {statistics.mean(rows_removed):.1f}")
                print(f"  Total Data Cleaned: {sum(bytes_removed):,} bytes")

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Benchmark on real datasets")
    parser.add_argument("--api-url", default="http://localhost:8080", help="Toolkit API URL")
    parser.add_argument("--datasets", help="Datasets directory")

    args = parser.parse_args()

    benchmark = RealDatasetBenchmark(
        datasets_dir=Path(args.datasets) if args.datasets else None,
        toolkit_api_url=args.api_url
    )
    benchmark.run_benchmark()
    benchmark.save_results()
