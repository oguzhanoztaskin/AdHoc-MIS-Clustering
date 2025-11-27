# Simple Ring Topology Demo

A clean, simple OMNeT++ project demonstrating a basic ring topology.

## Project Structure

- `RingNode.h/cc` - Simple ring node implementation
- `NetworkTopologies.ned` - Ring network topology definition
- `message.msg` - Simple message definition for ring communication
- `omnetpp.ini` - Simulation configuration

## How it Works

1. The network consists of nodes arranged in a ring topology
2. Each node is connected to exactly two neighbors (next and previous in the ring)
3. Node 0 initiates a token that travels around the ring
4. Each node forwards the token to the next node
5. The simulation completes when the token returns to the originator

## Running the Simulation

```bash
# Build the project
make

# Run with GUI
./demo

# Run from command line
./demo -c SimpleRing
```

## Configurations

- `SimpleRing`: 4 nodes
- `MediumRing`: 6 nodes  
- `LargeRing`: 8 nodes