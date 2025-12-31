#!/bin/bash

# Quick test script to verify everything is working
# Runs a small subset of tests and generates sample analysis

echo "========================================================================="
echo "           QUICK TEST - MIS Algorithm Evaluation Framework              "
echo "========================================================================="
echo ""

# Source OMNeT++ environment
source ~/Courses/adhoc/omnetpp-6.3.0/setenv 2>/dev/null

# Create test results directory
TEST_DIR="results_quicktest"
mkdir -p "$TEST_DIR"

echo "Test 1: FastMIS on complete graph (6 nodes, 5 runs)"
echo "-------------------------------------------------------------------"
./demo -u Cmdenv -c FastMIS-Complete -r 0..4 2>&1 | grep -E "(Run #|Simulation completed|INFO:)" | head -20

# Move results
mv results/*.sca "$TEST_DIR/" 2>/dev/null
mv results/*.vec "$TEST_DIR/" 2>/dev/null

echo ""
echo "Test 2: SlowMIS on complete graph (6 nodes, 5 runs)"
echo "-------------------------------------------------------------------"
./demo -u Cmdenv -c SlowMIS-Complete -r 0..4 2>&1 | grep -E "(Run #|Simulation completed|INFO:)" | head -20

# Move results
mv results/*.sca "$TEST_DIR/" 2>/dev/null
mv results/*.vec "$TEST_DIR/" 2>/dev/null

echo ""
echo "Test 3: FastMIS on 3x3 grid (5 runs)"
echo "-------------------------------------------------------------------"
./demo -u Cmdenv -c FastMIS-Grid-Small -r 0..4 2>&1 | grep -E "(Run #|Simulation completed|INFO:)" | head -15

# Move results
mv results/*.sca "$TEST_DIR/" 2>/dev/null
mv results/*.vec "$TEST_DIR/" 2>/dev/null

echo ""
echo "========================================================================="
echo "                        QUICK TEST COMPLETE                              "
echo "========================================================================="
echo ""

# Count results
RESULT_COUNT=$(ls -1 "$TEST_DIR"/*.sca 2>/dev/null | wc -l)
echo "Results collected: $RESULT_COUNT .sca files in $TEST_DIR/"

if [ $RESULT_COUNT -gt 0 ]; then
    echo ""
    echo "Now running analysis..."
    echo ""
    
    # Check if Python script exists
    if [ -f "analyze_results.py" ]; then
        python3 analyze_results.py "$TEST_DIR/"
        
        echo ""
        echo "========================================================================="
        echo "                    ANALYSIS COMPLETE                                    "
        echo "========================================================================="
        echo ""
        echo "✓ Plots generated in: analysis_output/"
        echo "✓ Tables generated in: analysis_output/"
        echo ""
        echo "Quick verification:"
        ls -lh analysis_output/*.png 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
        echo ""
        ls -lh analysis_output/*.csv 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
        echo ""
        echo "Next step: Run full simulation campaign with:"
        echo "  ./run_comprehensive_simulations.sh"
        echo ""
    else
        echo "⚠ Warning: analyze_results.py not found"
        echo "  Results are in $TEST_DIR/ but analysis cannot run"
    fi
else
    echo "⚠ Warning: No result files generated"
    echo "  Check that ./demo executable exists and runs correctly"
fi

echo "========================================================================="
