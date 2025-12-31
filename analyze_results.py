#!/usr/bin/env python3
"""
Comprehensive Analysis Script for MIS Clustering Algorithms
Generates publication-quality plots with 95% confidence intervals
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from scipy import stats
import sys
import glob
import re
from pathlib import Path

# Set style for publication-quality plots
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (10, 6)
plt.rcParams['font.size'] = 11
plt.rcParams['axes.labelsize'] = 12
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['xtick.labelsize'] = 10
plt.rcParams['ytick.labelsize'] = 10
plt.rcParams['legend.fontsize'] = 10

def parse_sca_file(filename):
    """Parse OMNeT++ .sca file and extract statistics"""
    data = {
        'config': '',
        'run': 0,
        'nodes': {}
    }
    
    current_module = None
    
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            
            # Extract configuration name
            if line.startswith('attr configname'):
                data['config'] = line.split()[-1]
            
            # Extract run number
            elif line.startswith('run'):
                parts = line.split()
                if len(parts) >= 2:
                    # Handle both "run 0" and "run FastMIS-Grid-Small-4-..." formats
                    try:
                        data['run'] = int(parts[1])
                    except ValueError:
                        # Extract from run name like "FastMIS-Grid-Small-4-..."
                        import re
                        match = re.search(r'-(\d+)-\d{8}', parts[1])
                        if match:
                            data['run'] = int(match.group(1))
                        else:
                            data['run'] = 0
            
            # Module declaration
            elif line.startswith('scalar'):
                parts = line.split()
                if len(parts) >= 3:
                    module_path = parts[1]
                    metric_name = parts[2]
                    value = float(parts[3]) if len(parts) > 3 else 0.0
                    
                    # Extract node ID from module path (e.g., "FastMISNetwork.node[5]")
                    match = re.search(r'node\[(\d+)\]', module_path)
                    if match:
                        node_id = int(match.group(1))
                        if node_id not in data['nodes']:
                            data['nodes'][node_id] = {}
                        data['nodes'][node_id][metric_name] = value
    
    return data

def calculate_confidence_interval(data, confidence=0.95):
    """Calculate mean and 95% confidence interval"""
    n = len(data)
    if n == 0:
        return 0, 0, 0
    
    mean = np.mean(data)
    if n == 1:
        return mean, mean, mean
    
    sem = stats.sem(data)  # Standard error of the mean
    ci = sem * stats.t.ppf((1 + confidence) / 2., n-1)
    
    return mean, mean - ci, mean + ci

def aggregate_metrics(sca_files, pattern):
    """Aggregate metrics from multiple .sca files matching a pattern"""
    results = []
    
    for sca_file in sca_files:
        if pattern.lower() not in str(sca_file).lower():
            continue
            
        data = parse_sca_file(sca_file)
        
        # Network-level metrics
        total_nodes = len(data['nodes'])
        mis_nodes = sum(1 for node in data['nodes'].values() if node.get('inMIS', 0) == 1)
        non_mis_nodes = total_nodes - mis_nodes
        
        # Message overhead metrics
        total_msg_sent = sum(node.get('totalMessagesSent', 0) for node in data['nodes'].values())
        total_msg_received = sum(node.get('totalMessagesReceived', 0) for node in data['nodes'].values())
        total_overhead = sum(node.get('totalMessageOverhead', 0) for node in data['nodes'].values())
        
        # Convergence time metrics
        convergence_times = [node.get('convergenceTime', 0) for node in data['nodes'].values() if node.get('convergenceTime', 0) > 0]
        max_convergence = max(convergence_times) if convergence_times else 0
        avg_convergence = np.mean(convergence_times) if convergence_times else 0
        
        # Phase metrics (FastMIS only)
        phases = [node.get('phase', 0) for node in data['nodes'].values() if node.get('phase', 0) > 0]
        max_phase = max(phases) if phases else 0
        
        # Neighbor metrics
        neighbor_counts = [node.get('initialNeighborCount', 0) for node in data['nodes'].values()]
        avg_neighbors = np.mean(neighbor_counts) if neighbor_counts else 0
        
        # Cluster size metrics (MIS node degree)
        mis_cluster_sizes = []
        for node_id, node_data in data['nodes'].items():
            if node_data.get('inMIS', 0) == 1:
                mis_cluster_sizes.append(node_data.get('initialNeighborCount', 0) + 1)  # +1 for the node itself
        
        avg_cluster_size = np.mean(mis_cluster_sizes) if mis_cluster_sizes else 0
        cluster_size_std = np.std(mis_cluster_sizes) if len(mis_cluster_sizes) > 1 else 0
        
        results.append({
            'config': data['config'],
            'run': data['run'],
            'total_nodes': total_nodes,
            'mis_nodes': mis_nodes,
            'non_mis_nodes': non_mis_nodes,
            'mis_ratio': mis_nodes / total_nodes if total_nodes > 0 else 0,
            'total_messages_sent': total_msg_sent,
            'total_messages_received': total_msg_received,
            'total_message_overhead': total_overhead,
            'avg_overhead_per_node': total_overhead / total_nodes if total_nodes > 0 else 0,
            'max_convergence_time': max_convergence,
            'avg_convergence_time': avg_convergence,
            'max_phase': max_phase,
            'avg_neighbors': avg_neighbors,
            'avg_cluster_size': avg_cluster_size,
            'cluster_size_std': cluster_size_std,
        })
    
    return pd.DataFrame(results)

def plot_metrics_with_ci(df, metric, ylabel, title, filename):
    """Create bar plot with 95% confidence intervals"""
    configs = df['config'].unique()
    
    means = []
    cis_lower = []
    cis_upper = []
    
    for config in configs:
        data = df[df['config'] == config][metric].values
        mean, ci_low, ci_high = calculate_confidence_interval(data)
        means.append(mean)
        cis_lower.append(mean - ci_low)
        cis_upper.append(ci_high - mean)
    
    fig, ax = plt.subplots(figsize=(12, 6))
    x = np.arange(len(configs))
    
    ax.bar(x, means, yerr=[cis_lower, cis_upper], capsize=5, alpha=0.7, 
           color='steelblue', edgecolor='black', linewidth=1.2)
    
    ax.set_xlabel('Configuration', fontweight='bold')
    ax.set_ylabel(ylabel, fontweight='bold')
    ax.set_title(title, fontweight='bold', pad=20)
    ax.set_xticks(x)
    ax.set_xticklabels(configs, rotation=45, ha='right')
    ax.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"✓ Generated: {filename}")

def plot_comparison(df, metric, ylabel, title, filename):
    """Create comparison plot between algorithms"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Group by algorithm type
    df['algorithm'] = df['config'].apply(lambda x: 'FastMIS' if 'FastMIS' in x else 'SlowMIS')
    
    algorithms = df['algorithm'].unique()
    x = np.arange(len(algorithms))
    
    means = []
    cis_lower = []
    cis_upper = []
    
    for algo in algorithms:
        data = df[df['algorithm'] == algo][metric].values
        mean, ci_low, ci_high = calculate_confidence_interval(data)
        means.append(mean)
        cis_lower.append(mean - ci_low)
        cis_upper.append(ci_high - mean)
    
    colors = ['#2E86AB', '#A23B72', '#F18F01']
    ax.bar(x, means, yerr=[cis_lower, cis_upper], capsize=8, alpha=0.8,
           color=colors[:len(algorithms)], edgecolor='black', linewidth=1.5)
    
    ax.set_xlabel('Algorithm', fontweight='bold')
    ax.set_ylabel(ylabel, fontweight='bold')
    ax.set_title(title, fontweight='bold', pad=20)
    ax.set_xticks(x)
    ax.set_xticklabels(algorithms)
    ax.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"✓ Generated: {filename}")

def generate_summary_table(df, output_file):
    """Generate LaTeX table with summary statistics"""
    configs = df['config'].unique()
    
    rows = []
    for config in configs:
        config_data = df[df['config'] == config]
        
        # Calculate metrics with CI
        metrics = {}
        for col in ['mis_nodes', 'total_message_overhead', 'avg_overhead_per_node', 
                    'max_convergence_time', 'avg_cluster_size']:
            mean, ci_low, ci_high = calculate_confidence_interval(config_data[col].values)
            metrics[col] = f"{mean:.2f} ({ci_low:.2f}-{ci_high:.2f})"
        
        rows.append({
            'Configuration': config,
            'MIS Nodes': metrics['mis_nodes'],
            'Total Overhead': metrics['total_message_overhead'],
            'Overhead/Node': metrics['avg_overhead_per_node'],
            'Convergence Time': metrics['max_convergence_time'],
            'Avg Cluster Size': metrics['avg_cluster_size']
        })
    
    summary_df = pd.DataFrame(rows)
    
    # Save as CSV
    csv_file = str(output_file).replace('.tex', '.csv')
    summary_df.to_csv(csv_file, index=False)
    
    # Generate LaTeX
    latex = summary_df.to_latex(index=False, escape=False, column_format='l' + 'c' * (len(summary_df.columns) - 1))
    
    with open(output_file, 'w') as f:
        f.write("% Summary Statistics with 95% Confidence Intervals\n")
        f.write("% Format: Mean (CI_lower-CI_upper)\n\n")
        f.write(latex)
    
    print(f"✓ Generated: {output_file}")
    print(f"✓ Generated: {csv_file}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python analyze_results.py <results_directory>")
        print("Example: python analyze_results.py results/")
        sys.exit(1)
    
    results_dir = Path(sys.argv[1])
    
    if not results_dir.exists():
        print(f"Error: Directory {results_dir} does not exist")
        sys.exit(1)
    
    # Find all .sca files
    sca_files = list(results_dir.glob("*.sca"))
    
    if not sca_files:
        print(f"No .sca files found in {results_dir}")
        sys.exit(1)
    
    print(f"\nFound {len(sca_files)} result files")
    print("=" * 60)
    
    # Create output directory
    output_dir = Path("analysis_output")
    output_dir.mkdir(exist_ok=True)
    
    # Analyze each algorithm separately
    for algo in ['FastMIS', 'SlowMIS']:
        print(f"\n{algo} Analysis:")
        print("-" * 60)
        
        df = aggregate_metrics(sca_files, algo)
        
        if df.empty:
            print(f"  No data found for {algo}")
            continue
        
        print(f"  Configurations: {df['config'].nunique()}")
        print(f"  Total runs: {len(df)}")
        
        # Generate plots
        plot_metrics_with_ci(df, 'mis_nodes', 'Number of MIS Nodes',
                            f'{algo}: MIS Size across Configurations',
                            output_dir / f'{algo}_mis_size.png')
        
        plot_metrics_with_ci(df, 'total_message_overhead', 'Total Messages',
                            f'{algo}: Message Overhead with 95% CI',
                            output_dir / f'{algo}_overhead.png')
        
        plot_metrics_with_ci(df, 'max_convergence_time', 'Time (seconds)',
                            f'{algo}: Convergence Time with 95% CI',
                            output_dir / f'{algo}_convergence.png')
        
        plot_metrics_with_ci(df, 'avg_cluster_size', 'Nodes per Cluster',
                            f'{algo}: Average Cluster Size with 95% CI',
                            output_dir / f'{algo}_cluster_size.png')
        
        # Generate summary table
        generate_summary_table(df, output_dir / f'{algo}_summary.tex')
    
    # Generate comparison plots
    print(f"\nGenerating Comparison Plots:")
    print("-" * 60)
    
    df_all = aggregate_metrics(sca_files, '')
    
    if not df_all.empty:
        plot_comparison(df_all, 'avg_overhead_per_node', 'Messages per Node',
                       'Algorithm Comparison: Overhead per Node',
                       output_dir / 'comparison_overhead.png')
        
        plot_comparison(df_all, 'max_convergence_time', 'Time (seconds)',
                       'Algorithm Comparison: Convergence Time',
                       output_dir / 'comparison_convergence.png')
        
        plot_comparison(df_all, 'mis_ratio', 'MIS/Total Nodes',
                       'Algorithm Comparison: MIS Ratio',
                       output_dir / 'comparison_mis_ratio.png')
    
    print("\n" + "=" * 60)
    print(f"Analysis complete! Results saved to {output_dir}/")
    print("=" * 60)

if __name__ == "__main__":
    main()
