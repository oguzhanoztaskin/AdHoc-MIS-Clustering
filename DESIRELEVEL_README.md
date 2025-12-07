# Desire-Level MIS Algorithm Implementation

This is an implementation of the Desire-Level MIS (Maximal Independent Set) algorithm as described in the paper "A Simple and Optimal Local Approximation Algorithm for MIS".

## Algorithm Overview

The Desire-Level MIS algorithm is an improvement over Luby's classical randomized MIS algorithm. It achieves better local complexity bounds by using a dynamic desire-level mechanism.

### Key Concepts

1. **Desire Level (p_t(v))**: Each node v maintains a desire level for joining the MIS, initially set to 0.5
   
2. **Effective Degree (d_t(v))**: The sum of desire levels of all neighbors of node v
   - Formula: `d_t(v) = Σ(p_t(u))` for all neighbors u of v

3. **Desire Level Update Rule**:
   - If `d_t(v) ≥ 2`: Decrease desire → `p_{t+1}(v) = p_t(v) / 2`
   - If `d_t(v) < 2`: Increase desire → `p_{t+1}(v) = min{2 * p_t(v), 0.5}`

4. **Marking and MIS Joining**:
   - In each round, node v gets marked with probability `p_t(v)`
   - If no neighbor is marked, v joins the MIS and terminates
   - Neighbors of MIS nodes also terminate

### Algorithm Phases per Round

Each round consists of two main communication phases:

**Phase 1: Exchange Desire Levels**
- Each node broadcasts its current desire level to all neighbors
- Nodes calculate their effective degree based on received values

**Phase 2: Marking and Decision**
- Each node marks itself with probability equal to its desire level
- Nodes exchange marking status
- If a node is marked and no neighbor is marked, it joins the MIS
- Nodes with MIS neighbors terminate

## Complexity Analysis

- **Local Complexity**: O(log Δ + log 1/ε) rounds
  - Δ = maximum degree in the graph
  - ε = failure probability
- **Message Size**: 2 bits per message (for desire level changes)
- **Communication Rounds per Algorithm Round**: 2

## Implementation Details

### Files

- `DesireLevelMISNode.h/cc`: Main node implementation
- `message.msg`: Message definitions (MISDesireLevelMessage, MISMarkMessage)
- `NetworkTopologies.ned`: Network topology definitions
- `omnetpp.ini`: Simulation configurations

### Key Features

1. **Visual Feedback**:
   - Blue laptop icon: Active node
   - Green server icon: Node in MIS
   - Red PC icon: Terminated (not in MIS)

2. **Statistics Recorded**:
   - `rounds`: Number of rounds until termination
   - `inMIS`: Whether node is in MIS (1) or not (0)
   - `finalDesireLevel`: Final desire level value

3. **Configurable Parameters**:
   - `roundInterval`: Time between rounds (default: 2.0s)
   - `initialStartDelay`: Random start delay (default: 0.1s)
   - `desireLevelSendDelay`: Delay before sending (default: 0.1s)

## Running Simulations

### Build the Project

```bash
source /home/oguzhan/Courses/ad-hoc/omnetpp-6.2.0/setenv
cd /home/oguzhan/Courses/ad-hoc/omnetpp-6.2.0/adhoc-project/demo
make clean && make
```

### Run Tests

```bash
# Run the test script
./test_desirelevel.sh

# Or run individual configurations
./demo -u Cmdenv -c DesireLevelMIS-Complete -r 0
./demo -u Cmdenv -c DesireLevelMIS-Grid-Small -r 0
./demo -u Cmdenv -c DesireLevelMIS-Random-Small-Sparse -r 0
```

### Available Configurations

1. **Complete Graphs**:
   - `DesireLevelMIS-Complete`: 6 nodes
   - `DesireLevelMIS-Complete-Large`: 10 nodes

2. **Grid Topologies**:
   - `DesireLevelMIS-Grid-Small`: 3×3 grid
   - `DesireLevelMIS-Grid-Medium`: 4×4 grid

3. **Random Graphs** (Erdős-Rényi):
   - `DesireLevelMIS-Random-Small-Sparse`: 20 nodes, p=0.2
   - `DesireLevelMIS-Random-Small-Dense`: 20 nodes, p=0.6
   - `DesireLevelMIS-Random-Large-Sparse`: 50 nodes, p=0.1
   - `DesireLevelMIS-Random-Large-Medium`: 50 nodes, p=0.3
   - `DesireLevelMIS-Random-Large-Dense`: 50 nodes, p=0.6

### Custom Parameters

```bash
# Run with custom graph parameters
./demo -u Cmdenv -c DesireLevelMIS-RandomGraph \
  --*.numNodes=100 \
  --*.edgeProbability=0.4 \
  --*.node[*].roundInterval=2.0 \
  -r 0-99
```

## Advantages Over Classical Algorithms

1. **Better Local Complexity**: O(log Δ + log 1/ε) vs O(log² Δ) for basic Luby
2. **Improved ε-dependency**: Linear in log(1/ε) instead of log Δ · log(1/ε)
3. **Simple and Clean**: Deterministic desire-level dynamics
4. **Decentralized**: Decisions based only on 2-hop neighborhood

## Algorithm Intuition

The algorithm creates two stable scenarios:

1. **Low Competition**: Node has low effective degree (< 2), so it increases its desire level and tries to join MIS
2. **High Neighbor Activity**: Node has high effective degree (≥ 2), indicating many neighbors want to join, so it decreases its desire and waits

This dynamic ensures nodes spend significant time in scenarios where they can either:
- Join the MIS themselves (when competition is low)
- Have a neighbor join the MIS (when neighbor activity is high)

## Comparison with Other Implementations

The project includes three MIS algorithms for comparison:

1. **SlowMIS**: Simple ID-based algorithm (Θ(n) worst case)
2. **FastMIS**: Luby's randomized algorithm with phases (O(log Δ · log log Δ))
3. **DesireLevelMIS**: This implementation (O(log Δ + log 1/ε))

## References

This implementation is based on the paper describing a simple and optimal local approximation algorithm for MIS with improved complexity bounds.

## Output Example

```
INFO: DesireLevelMIS Node 0 finished in 5 rounds. IN MIS (final desire level: 0.500000)
INFO: DesireLevelMIS Node 1 finished in 5 rounds. NOT in MIS (final desire level: 0.125000)
INFO: DesireLevelMIS Node 2 finished in 5 rounds. NOT in MIS (final desire level: 0.250000)
```

## Troubleshooting

1. **Build Errors**: Make sure to source the OMNeT++ environment first
   ```bash
   source /home/oguzhan/Courses/ad-hoc/omnetpp-6.2.0/setenv
   ```

2. **Message Compilation Issues**: If messages don't compile, try:
   ```bash
   make clean && make
   ```

3. **Simulation Hangs**: Check `roundInterval` parameter - increase if needed for debugging
