# MIS Clustering Algorithms - Comprehensive Evaluation Framework

This repository implements and evaluates Maximum Independent Set (MIS) clustering algorithms for ad-hoc networks with a **complete methodology framework suitable for thesis research**.

## 🎯 What's New - December 2025 Update

### ✅ **Comprehensive Statistics Collection**

- 9+ metrics tracked per node (overhead, convergence, cluster quality)
- Network-wide aggregate statistics
- OMNeT++ signal emission for real-time analysis

### ✅ **Professional Analysis Tools**

- Python script with 95% confidence intervals
- Publication-quality plots (300 DPI)
- LaTeX table generation
- CSV exports for spreadsheets

### ✅ **Extensive Simulation Campaign**

- 37+ test configurations
- 1100+ simulation runs
- Scalability tests (20-1000 nodes)
- Density variation (10%-90%)
- Automated execution scripts

## 🚀 Quick Start

### For Verification (2 minutes):

```bash
source ~/Courses/adhoc/omnetpp-6.3.0/setenv
make clean && make
./quick_test.sh
```

### For Complete Analysis (2-4 hours):

```bash
./run_comprehensive_simulations.sh
# Wait for completion...
python3 analyze_results.py results_comprehensive/
```

Results will be in `analysis_output/` with plots and tables ready for your report!

## 📚 Documentation

| File                                                       | Description                            |
| ---------------------------------------------------------- | -------------------------------------- |
| **[QUICKSTART.md](QUICKSTART.md)**                         | Get started in 5 minutes               |
| **[METHODOLOGY.md](METHODOLOGY.md)**                       | Complete methodology for thesis report |
| **[IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)** | What was implemented and why           |

## 📊 Metrics Evaluated

### Cluster Quality

- Number of MIS nodes (cluster heads)
- Cluster size distribution
- Balanced vs unbalanced clustering

### Overhead

- Total messages sent/received
- Control message overhead
- Messages per node
- Messages per phase

### Performance

- Convergence time (max and average)
- Algorithm phases (FastMIS)
- Time to network organization

### Scalability

- Performance vs network size (20-1000 nodes)
- Performance vs density (10%-90%)
- Stress test results

## 🔬 Algorithms Implemented

1. **FastMIS**: Randomized distributed MIS with phases
2. **SlowMIS**: ID-based deterministic MIS

Each with comprehensive statistics tracking and analysis support.

## 📈 Generated Outputs

After running simulations and analysis:

```
analysis_output/
├── FastMIS_mis_size.png          # MIS size across configs
├── FastMIS_overhead.png          # Message overhead with CI
├── FastMIS_convergence.png       # Convergence time with CI
├── FastMIS_cluster_size.png      # Cluster size with CI
├── SlowMIS_*.png                 # SlowMIS metrics
├── comparison_*.png              # Algorithm comparisons
├── FastMIS_summary.tex          # LaTeX table
├── FastMIS_summary.csv          # Spreadsheet data
└── ...
```

All plots include 95% confidence intervals from 30 runs per configuration.

## 🎓 Using in Your Thesis

### Methodology Chapter

- Copy methodology text from `METHODOLOGY.md`
- Cite: OMNeT++ 6.3.0, 37+ configurations, 30 runs, 95% CI
- Describe: Topology variation, scalability, density analysis

### Results Chapter

- Include plots from `analysis_output/`
- Include LaTeX tables from `analysis_output/*.tex`
- Discuss: Algorithm comparison, scalability, optimal ranges

### Discussion Chapter

- Analyze: Trade-offs (speed vs overhead)
- Evaluate: Which algorithm for which scenario
- Recommend: Best practices for deployment

## 🛠️ Key Files

| File                               | Purpose                           |
| ---------------------------------- | --------------------------------- |
| `FastMISNode.cc/h`                 | FastMIS implementation with stats |
| `SlowMISNode.cc/h`                 | SlowMIS implementation with stats |
| `omnetpp.ini`                      | 50+ test configurations           |
| `analyze_results.py`               | Statistical analysis & plotting   |
| `run_comprehensive_simulations.sh` | Automated test runner             |
| `quick_test.sh`                    | Fast verification script          |

## ⚙️ Requirements

- OMNeT++ 6.2.0 or 6.3.0
- Python 3.7+
- Python packages: pandas, numpy, matplotlib, seaborn, scipy
- Linux/Unix environment

Install Python dependencies:

```bash
pip install pandas numpy matplotlib seaborn scipy
```

## 📋 Test Matrix

- **Topologies**: Complete graphs, grids, random (Erdős-Rényi)
- **Node Counts**: 6, 10, 20, 50, 100, 200, 500, 1000
- **Densities**: 0.1, 0.3, 0.5, 0.7, 0.9
- **Runs per Config**: 30
- **Total Simulations**: 1100+

## ⚡ Performance

- Quick test: 2 minutes
- Full campaign: 2-4 hours
- Analysis: 1-2 minutes
- Results size: ~100MB

## 📊 Statistical Rigor

- **Confidence Intervals**: 95% using Student's t-distribution
- **Sample Size**: 30 runs per configuration
- **Error Bars**: Shown on all plots
- **Significance**: Proper statistical methodology

## 🎯 What This Shows

✅ Professional methodology framework  
✅ Rigorous statistical analysis  
✅ Publication-quality outputs  
✅ Comprehensive test coverage  
✅ Reproducible results  
✅ Ready for thesis report

## 🚧 Limitations

This implementation focuses on **algorithmic evaluation** without:

- Mobility models (requires INET framework)
- Energy models (requires INET framework)
- Application layer (requires INET framework)
- Routing protocols (requires INET framework)

These represent orthogonal research dimensions and are reserved for future work.

## 📞 Getting Help

1. Read [QUICKSTART.md](QUICKSTART.md) for step-by-step instructions
2. Read [METHODOLOGY.md](METHODOLOGY.md) for detailed explanations
3. Check [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) for technical details

## 🎓 Citation

When using this framework in your research:

```
MIS Clustering Algorithms Evaluation Framework
OMNeT++ Implementation with Comprehensive Methodology
December 2025
```

---

**Status**: ✅ Complete and ready for thesis report generation  
**Last Updated**: December 29, 2025  
**Framework Version**: 2.0 (Comprehensive Evaluation)
