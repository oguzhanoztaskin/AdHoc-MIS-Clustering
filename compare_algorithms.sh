#!/bin/bash

# Script to compare all three MIS algorithms

# Source OMNeT++ environment
source /home/oguzhan/Courses/ad-hoc/omnetpp-6.2.0/setenv

echo "================================================"
echo "Comparing MIS Algorithms on 3x3 Grid"
echo "================================================"
echo ""

echo "1. SlowMIS (ID-based algorithm)"
echo "----------------------------------------"
./demo -u Cmdenv -c SlowMIS-Grid-Small -r 0 2>&1 | grep "INFO:" | head -10
echo ""

echo "2. FastMIS (Luby's with phases)"
echo "----------------------------------------"
./demo -u Cmdenv -c FastMIS-Grid-Small -r 0 2>&1 | grep "INFO:" | head -10
echo ""

echo "================================================"
echo "Comparison complete!"
echo "================================================"
echo ""
echo "Key observations:"
echo "- SlowMIS: Deterministic, depends on node IDs"
echo "- FastMIS: Randomized phases, O(log Δ · log log Δ)"
