#!/bin/bash

# Comprehensive simulation campaign for MIS clustering algorithms
# Runs extensive tests across multiple topologies and conditions

# Source OMNeT++ environment
source ~/Courses/adhoc/omnetpp-6.3.0/setenv 2>/dev/null || source /home/oguzhan/Courses/ad-hoc/omnetpp-6.2.0/setenv

echo "========================================================================="
echo "           MIS CLUSTERING ALGORITHMS - SIMULATION CAMPAIGN             "
echo "========================================================================="
echo ""

# Configuration
RUNS="0..29"  # 30 runs for good confidence intervals
RESULTS_DIR="results_comprehensive"

# Create results directory
mkdir -p "$RESULTS_DIR"

# Counter for progress
total_tests=0
completed_tests=0

# Function to run simulation and move results
run_simulation() {
    local config=$1
    local description=$2
    
    total_tests=$((total_tests + 1))
    
    echo ""
    echo "-------------------------------------------------------------------------"
    echo "Test $total_tests: $description"
    echo "Config: $config"
    echo "Runs: $RUNS"
    echo "-------------------------------------------------------------------------"
    
    ./demo -u Cmdenv -c "$config" -r "$RUNS" 2>&1 | grep -E "(INFO:|Run #|Simulation completed|Error)"
    
    # Move result files
    for file in results/*.sca results/*.vec; do
        if [ -f "$file" ]; then
            mv "$file" "$RESULTS_DIR/"
        fi
    done
    
    completed_tests=$((completed_tests + 1))
    echo "✓ Completed test $completed_tests/$total_tests"
}

echo "Starting simulation campaign at $(date)"
echo ""

# =============================================================================
# TOPOLOGY VARIATION TESTS
# =============================================================================

echo ""
echo "========================================================================="
echo "PART 1: TOPOLOGY VARIATION"
echo "========================================================================="

# Complete Graph Tests (fully connected)
run_simulation "FastMIS-Complete" "FastMIS on complete graph (6 nodes)"
run_simulation "SlowMIS-Complete" "SlowMIS on complete graph (6 nodes)"

run_simulation "FastMIS-Complete-Large" "FastMIS on complete graph (10 nodes)"
run_simulation "SlowMIS-Complete-Large" "SlowMIS on complete graph (10 nodes)"

# Grid Topology Tests (structured)
run_simulation "FastMIS-Grid-Small" "FastMIS on 3x3 grid"
run_simulation "SlowMIS-Grid-Small" "SlowMIS on 3x3 grid"

run_simulation "FastMIS-Grid-Medium" "FastMIS on 4x4 grid"
run_simulation "SlowMIS-Grid-Medium" "SlowMIS on 4x4 grid"

# =============================================================================
# SCALABILITY TESTS (varying node count)
# =============================================================================

echo ""
echo "========================================================================="
echo "PART 2: SCALABILITY ANALYSIS"
echo "========================================================================="

NODE_COUNTS=(20 50 100 200)
EDGE_PROB=0.3  # Medium density

for NODES in "${NODE_COUNTS[@]}"; do
    run_simulation "FastMIS-RandomGraph" "FastMIS - $NODES nodes (p=$EDGE_PROB)" \
        "--*.numNodes=$NODES --*.edgeProbability=$EDGE_PROB"
    
    run_simulation "SlowMIS-RandomGraph" "SlowMIS - $NODES nodes (p=$EDGE_PROB)" \
        "--*.numNodes=$NODES --*.edgeProbability=$EDGE_PROB"
done

# =============================================================================
# DENSITY VARIATION TESTS
# =============================================================================

echo ""
echo "========================================================================="
echo "PART 3: NETWORK DENSITY ANALYSIS"
echo "========================================================================="

NODES=100
DENSITIES=(0.1 0.3 0.5 0.7 0.9)

for PROB in "${DENSITIES[@]}"; do
    density_percent=$(echo "$PROB * 100" | bc)
    
    run_simulation "FastMIS-RandomGraph" "FastMIS - density ${density_percent}% ($NODES nodes)" \
        "--*.numNodes=$NODES --*.edgeProbability=$PROB"
    
    run_simulation "SlowMIS-RandomGraph" "SlowMIS - density ${density_percent}% ($NODES nodes)" \
        "--*.numNodes=$NODES --*.edgeProbability=$PROB"
done

# =============================================================================
# STRESS TESTS (extreme conditions)
# =============================================================================

echo ""
echo "========================================================================="
echo "PART 4: STRESS TESTS"
echo "========================================================================="

# Very sparse network
run_simulation "FastMIS-RandomGraph" "FastMIS - Very sparse (500 nodes, p=0.05)" \
    "--*.numNodes=500 --*.edgeProbability=0.05"

# Very dense small network
run_simulation "FastMIS-RandomGraph" "FastMIS - Very dense (50 nodes, p=0.95)" \
    "--*.numNodes=50 --*.edgeProbability=0.95"

# Large sparse network
run_simulation "FastMIS-RandomGraph" "FastMIS - Large sparse (1000 nodes, p=0.01)" \
    "--*.numNodes=1000 --*.edgeProbability=0.01"

# =============================================================================
# SUMMARY
# =============================================================================

echo ""
echo "========================================================================="
echo "                      SIMULATION CAMPAIGN COMPLETE                      "
echo "========================================================================="
echo ""
echo "Summary:"
echo "  Total tests completed: $completed_tests"
echo "  Results directory: $RESULTS_DIR/"
echo "  Completion time: $(date)"
echo ""
echo "Next steps:"
echo "  1. Run analysis: python3 analyze_results.py $RESULTS_DIR/"
echo "  2. View plots in: analysis_output/"
echo "  3. LaTeX tables in: analysis_output/*.tex"
echo ""
echo "========================================================================="
