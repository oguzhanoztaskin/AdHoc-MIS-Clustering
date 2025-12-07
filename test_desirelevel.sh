#!/bin/bash

# Script to test the Desire-Level MIS algorithm implementation

# Source OMNeT++ environment
source /home/oguzhan/Courses/ad-hoc/omnetpp-6.2.0/setenv

echo "================================"
echo "Testing Desire-Level MIS Algorithm"
echo "================================"
echo ""

# Test 1: Small complete graph
echo "Test 1: Small complete graph (6 nodes)"
./demo -u Cmdenv -c DesireLevelMIS-Complete -r 0
echo ""

# Test 2: Grid topology
echo "Test 2: 3x3 Grid topology"
./demo -u Cmdenv -c DesireLevelMIS-Grid-Small -r 0
echo ""

# Test 3: Random sparse graph
echo "Test 3: Random sparse graph (20 nodes, p=0.2)"
./demo -u Cmdenv -c DesireLevelMIS-Random-Small-Sparse -r 0
echo ""

# Test 4: Random dense graph
echo "Test 4: Random dense graph (20 nodes, p=0.6)"
./demo -u Cmdenv -c DesireLevelMIS-Random-Small-Dense -r 0
echo ""

echo "================================"
echo "All tests completed!"
echo "================================"
