#!/bin/bash

# Script to generate CSV with raw results on the random graphs
# Usage: ./random_graph_report.sh [runs] [output_file]
# Example: ./random_graph_report.sh "0..10" results.csv
# Example: ./random_graph_report.sh "0,1,2,5,10,20"
#
# Tests multiple combinations of node counts and edge probabilities:
# - Nodes: 100, 1000, 2000
# - Edge Probabilities: 0.2, 0.5, 0.8

# Default values
DEFAULT_RUNS="0..10"
DEFAULT_OUTPUT="random_graph_results.csv"

# Node counts and edge probabilities to test
NODE_COUNTS=(100 1000 2000)
EDGE_PROBS=(0.2 0.5 0.8)

# Parse command line arguments
RUNS=${1:-$DEFAULT_RUNS}
CSV_FILE=${2:-$DEFAULT_OUTPUT}

echo "Running simulations with:"
echo "  Node counts: ${NODE_COUNTS[@]}"
echo "  Edge probabilities: ${EDGE_PROBS[@]}"
echo "  Runs: $RUNS"
echo "  Output: $CSV_FILE"
echo ""

# Header
echo "Algorithm,Nodes,EdgeProb,Seed,EventCount,SimTime,MessageCount" > "$CSV_FILE"

# Run all combinations
for NODES in "${NODE_COUNTS[@]}"; do
    for EDGE_PROB in "${EDGE_PROBS[@]}"; do
        echo "=== Testing: $NODES nodes, edge probability $EDGE_PROB ==="
        
        for algo in FastMIS SlowMIS; do
            config="${algo}-RandomGraph"
            echo "  Running $config..."
            
            # Run simulation with parameters
            OUTPUT=$(./demo -c "$config" -u Cmdenv \
                --*.numNodes="$NODES" \
                --*.edgeProbability="$EDGE_PROB" \
                -r "$RUNS" 2>&1)
            
            # Parse results
            current_seed=""
            event_count=""
            sim_time=""
            msg_count=""
            
            while IFS= read -r line; do
                # New scenario - save previous and start new (look for "run #N")
                if [[ $line =~ run\ \#([0-9]+) ]]; then
                    if [[ -n "$current_seed" && -n "$event_count" && -n "$sim_time" && -n "$msg_count" ]]; then
                        echo "$algo,$NODES,$EDGE_PROB,$current_seed,$event_count,$sim_time,$msg_count" >> "$CSV_FILE"
                    fi
                    current_seed="${BASH_REMATCH[1]}"
                    event_count=""
                    sim_time=""
                    msg_count=""
                fi
                
                # Extract event count and sim time (format: at t=0.189460666385s, event #129)
                if [[ $line =~ at\ t=([0-9.]+)s,\ event\ \#([0-9]+) ]]; then
                    sim_time="${BASH_REMATCH[1]}"
                    event_count="${BASH_REMATCH[2]}"
                fi
                
                # Extract message count (format: Messages:  created: 152   present: 20   in FES: 0)
                if [[ $line =~ Messages:.*created:\ +([0-9]+) ]]; then
                    msg_count="${BASH_REMATCH[1]}"
                fi
            done < <(echo "$OUTPUT")
            
            # Save last run
            if [[ -n "$current_seed" && -n "$event_count" && -n "$sim_time" && -n "$msg_count" ]]; then
                echo "$algo,$NODES,$EDGE_PROB,$current_seed,$event_count,$sim_time,$msg_count" >> "$CSV_FILE"
            fi
        done
        echo ""
    done
done

echo "================================"
echo "All simulations completed!"
echo "Results saved to: $CSV_FILE"
