# Dissertation Visualization Guide

Convert benchmark results into publication-ready charts for your honours project.

## Generated Visualizations

### 1. performance_comparison.png
- **Type**: Box plot with statistics overlay
- **Shows**: Execution time variance across datasets
- **Key Metrics**: Mean, median, standard deviation
- **Use Case**: Demonstrate Toolkit's speed advantage

### 2. effectiveness_comparison.png
- **Type**: Side-by-side bar charts
- **Shows**: Average data reduction (bytes and rows)
- **Key Metrics**: Cleaning intensity per dataset
- **Use Case**: Show how much data each tool cleans

### 3. dataset_breakdown.png
- **Type**: Line plots (dataset-by-dataset)
- **Shows**: Performance and effectiveness across all 103 datasets
- **Key Metrics**: Trends across different data types (Airbnb, Credit, Titanic)
- **Use Case**: Comprehensive comparison across test suite

### 4. summary_statistics.png
- **Type**: Statistics table
- **Shows**: Aggregate metrics (counts, means, totals)
- **Key Metrics**: Overall performance summary
- **Use Case**: Reference table for dissertation text

## How to Generate

### Basic Run
```bash
python3 visualize_openrefine_comparison.py
```

### Custom Paths
```bash
python3 visualize_openrefine_comparison.py \
  --results benchmark_results/openrefine_comparison_results.json \
  --output benchmark_results/
```

## Requirements

```bash
pip install matplotlib numpy
```

## Using in LaTeX

### Include single chart
```latex
\begin{figure}[h]
  \centering
  \includegraphics[width=0.9\textwidth]{benchmark_results/performance_comparison.png}
  \caption{Execution time comparison: Data Cleaning Toolkit vs OpenRefine across 103 datasets.}
  \label{fig:performance-comparison}
\end{figure}
```

### Include all comparisons
```latex
\subsection{Experimental Results}

\begin{figure}[h]
  \centering
  \includegraphics[width=1\textwidth]{benchmark_results/performance_comparison.png}
  \caption{Performance comparison showing execution time distribution.}
  \label{fig:perf}
\end{figure}

\begin{figure}[h]
  \centering
  \includegraphics[width=1\textwidth]{benchmark_results/effectiveness_comparison.png}
  \caption{Data reduction effectiveness: average bytes and rows removed per dataset.}
  \label{fig:effective}
\end{figure}

\begin{figure}[h]
  \centering
  \includegraphics[width=1\textwidth]{benchmark_results/dataset_breakdown.png}
  \caption{Dataset-by-dataset analysis across Airbnb, Credit, and Titanic datasets.}
  \label{fig:breakdown}
\end{figure}

\begin{figure}[h]
  \centering
  \includegraphics[width=1\textwidth]{benchmark_results/summary_statistics.png}
  \caption{Summary statistics table of benchmark results.}
  \label{fig:summary}
\end{figure}
```

## Customization Options

Edit `visualize_openrefine_comparison.py` to:

### Change colors
```python
COLORS = {
    'Toolkit': '#0000ee',      # Blue
    'OpenRefine': '#ff6b6b',   # Red
}
```

### Change figure size
```python
fig, ax = plt.subplots(figsize=(14, 8))  # Width, Height
```

### Change DPI (print quality)
```python
plt.savefig(filepath, dpi=600)  # Higher DPI = sharper
```

### Change fonts
```python
plt.rcParams['font.family'] = 'serif'  # For formal dissertations
```

## Interpretation Guide

### For your dissertation text:

**Performance Section:**
> "The Data Cleaning Toolkit demonstrated [X]% faster execution time compared to OpenRefine, with mean execution time of [Y]ms vs [Z]ms."

**Effectiveness Section:**
> "Across 103 datasets, the Toolkit removed an average of [X] bytes per dataset, compared to OpenRefine's [Y] bytes, indicating [more/less] aggressive cleaning strategies."

**Scope Section:**
> "While OpenRefine is CSV-focused, the Toolkit extends to JSON and XML formats, providing a [broader/narrower] cleaning scope suitable for [use case]."

## Data Integrity Notes

- All visualizations are derived from `openrefine_comparison_results.json`
- Original datasets remain unchanged
- Results are reproducible: same input → same output
- Charts are publication-ready at 300 DPI

## File Organization

```
benchmark_results/
├── openrefine_comparison_results.json    (Raw data)
├── openrefine_comparison_results.csv     (Spreadsheet view)
├── openrefine_comparison_report.json     (Statistics)
├── performance_comparison.png            (Chart)
├── effectiveness_comparison.png          (Chart)
├── dataset_breakdown.png                 (Chart)
└── summary_statistics.png                (Chart)
```
