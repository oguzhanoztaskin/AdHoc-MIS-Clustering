# Ad-Hoc Network Algorithms Demo

An OMNeT++ project demonstrating various distributed algorithms for ad-hoc networks.

## Algorithms Implemented

### 1. Ring Token Passing
A simple ring topology where a token circulates between nodes.

### 2. Fast MIS (Maximal Independent Set)
A randomized algorithm where nodes compete to join the MIS using random values.
- Uses multiple phases with random value generation
- Nodes with smallest random values among their neighbors join the MIS
- Fast convergence through probabilistic decisions

### 3. Slow MIS (Maximal Independent Set) - Algorithm 7.3
A deterministic algorithm based on node IDs:
- **Algorithm**: A node joins the MIS if all neighbors with larger IDs have decided not to join
- Deterministic behavior based purely on node topology and IDs
- Slower convergence but predictable results

## Project Structure

- `RingNode.h/cc` - Simple ring node implementation
- `FastMISNode.h/cc` - Fast MIS algorithm implementation  
- `SlowMISNode.h/cc` - Slow MIS algorithm implementation
- `NetworkTopologies.ned` - All network topology definitions
- `message.msg` - Message definitions for all algorithms
- `omnetpp.ini` - Simulation configurations for all algorithms

## Running the Simulations

```bash
# Build the project
make

# Run examples script to see all available configurations
./run_examples.sh

# Example runs:
./demo -c FastMIS-Grid-Small -u Cmdenv --sim-time-limit=10s
./demo -c SlowMIS-Complete -u Cmdenv --sim-time-limit=10s
./demo -c SimpleRing -u Cmdenv --sim-time-limit=10s
```

## Available Configurations

### Fast MIS Algorithm:
- `FastMIS-Complete` - Complete graph with 6 nodes
- `FastMIS-Complete-Large` - Complete graph with 10 nodes
- `FastMIS-Grid-Small` - 3x3 grid topology
- `FastMIS-Grid-Medium` - 4x4 grid topology
- `FastMIS-Fast` - Fast execution with shorter timeouts

### Slow MIS Algorithm:
- `SlowMIS-Complete` - Complete graph with 6 nodes
- `SlowMIS-Complete-Large` - Complete graph with 10 nodes
- `SlowMIS-Grid-Small` - 3x3 grid topology
- `SlowMIS-Grid-Medium` - 4x4 grid topology
- `SlowMIS-Fast` - Fast execution with shorter check interval

### Ring Algorithm:
- `SimpleRing` - Ring with 4 nodes
- `MediumRing` - Ring with 6 nodes
- `LargeRing` - Ring with 8 nodes

## Algorithm Comparison

**Fast MIS vs Slow MIS:**

- **Fast MIS**: Uses randomization, faster convergence, non-deterministic results
- **Slow MIS**: Uses node IDs, deterministic results, potentially slower convergence

**Network Topologies:**

- **Complete Graph**: Every node connected to every other node
- **Grid Topology**: Nodes arranged in a rectangular grid with local connectivity
- **Ring Topology**: Nodes connected in a circular chain

## Results Analysis

The algorithms generate `.sca` result files in the `results/` directory that can be analyzed for:
- Number of nodes in MIS
- Algorithm convergence time
- Message complexity
- Phase counts (for Fast MIS)