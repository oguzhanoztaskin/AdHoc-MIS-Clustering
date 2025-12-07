#!/bin/bash

# Script to generate CSV with raw results on the random graphs
CSV_FILE="random_graph_results.csv"

# Header
echo "Algorithm,Nodes,EdgeProb,Seed,EventCount,SimTime,MessageCount" > "$CSV_FILE"

# Run all configurations
for algo in FastMIS SlowMIS; do
    for size in Small Medium Large; do
        for density in Sparse Medium Dense; do
            config="${algo}-Random-${size}-${density}"
            
            # Determine node count
            if [[ $size == "Small" ]]; then
                nodes=10
            elif [[ $size == "Medium" ]]; then
                nodes=50
            else
                nodes=100
            fi
            
            # Determine edge probability
            if [[ $density == "Sparse" ]]; then
                edge_prob=0.2
            elif [[ $density == "Medium" ]]; then
                edge_prob=0.5
            else
                edge_prob=0.8
            fi
            
            # Run simulation
            OUTPUT=$(./demo -c "$config" -u Cmdenv 2>&1)
            
            # Parse results
            current_seed=""
            event_count=""
            sim_time=""
            msg_count=""
            
            while IFS= read -r line; do
                # New scenario - save previous and start new
                if [[ $line =~ Scenario:.*\$0=([0-9]+) ]]; then
                    if [[ -n "$current_seed" && -n "$event_count" && -n "$sim_time" && -n "$msg_count" ]]; then
                        echo "$algo,$nodes,$edge_prob,$current_seed,$event_count,$sim_time,$msg_count" >> "$CSV_FILE"
                    fi
                    current_seed="${BASH_REMATCH[1]}"
                    event_count=""
                    sim_time=""
                    msg_count=""
                fi
                
                # Extract event count and sim time
                if [[ $line =~ No\ more\ events.*at\ t=([0-9.]+)s.*event\ \#([0-9]+) ]]; then
                    sim_time="${BASH_REMATCH[1]}"
                    event_count="${BASH_REMATCH[2]}"
                fi
                
                # Extract message count
                if [[ $line =~ Messages:\ +created:\ +([0-9]+) ]]; then
                    msg_count="${BASH_REMATCH[1]}"
                fi
            done < <(echo "$OUTPUT")
            
            # Save last run
            if [[ -n "$current_seed" && -n "$event_count" && -n "$sim_time" && -n "$msg_count" ]]; then
                echo "$algo,$nodes,$edge_prob,$current_seed,$event_count,$sim_time,$msg_count" >> "$CSV_FILE"
            fi
        done
    done
done

echo "Results saved to: $CSV_FILE"
