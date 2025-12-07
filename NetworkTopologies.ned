//
// Network Topologies for Ring and Fast MIS algorithms
//

// Simple node for ring topology
simple RingNode
{
    parameters:
        int nodeId = default(index);
        @class(RingNode);
        @display("i=device/laptop");
    gates:
        input in[2];   // Two inputs: from left and right neighbors
        output out[2]; // Two outputs: to left and right neighbors
}

// Fast MIS node for general graph topologies
simple FastMISNode
{
    parameters:
        int nodeId = default(index);
        double phaseInterval = default(2.0);        // Time between phases
        double initialStartDelay = default(0.1);    // Max delay for initial algorithm start
        double randomValueSendDelay = default(0.1); // Delay before sending random value
        @class(FastMISNode);
        @display("i=device/server;is=s");
    gates:
        input in[];   // Variable number of inputs
        output out[]; // Variable number of outputs
}

// Slow MIS node for general graph topologies
simple SlowMISNode
{
    parameters:
        int nodeId = default(index);
        double initialStartDelay = default(0.1);    // Max delay for initial algorithm start
        @class(SlowMISNode);
        @display("i=device/laptop;is=s");
    gates:
        input in[];   // Variable number of inputs
        output out[]; // Variable number of outputs
}

// Desire-Level MIS node for general graph topologies
simple DesireLevelMISNode
{
    parameters:
        int nodeId = default(index);
        double roundInterval = default(2.0);        // Time between rounds
        double initialStartDelay = default(0.1);    // Max delay for initial algorithm start
        double desireLevelSendDelay = default(0.1); // Delay before sending desire level
        @class(DesireLevelMISNode);
        @display("i=device/laptop;is=s");
    gates:
        input in[];   // Variable number of inputs
        output out[]; // Variable number of outputs
}

// Ring topology network
network RingNetwork
{
    parameters:
        int numNodes = default(6);
        @display("bgb=400,400");

    submodules:
        node[numNodes]: RingNode {
            parameters:
                nodeId = index;
                @display("p=200+150*cos(2*3.14159*index/numNodes),200+150*sin(2*3.14159*index/numNodes)");
        }

    connections:
        // Connect each node to next and previous node in ring
        for i=0..numNodes-1 {
            node[i].out[0] --> node[(i+1)%numNodes].in[0];  // to next node
            node[i].out[1] --> node[(i+numNodes-1)%numNodes].in[1];  // to previous node
        }
}

// Fast MIS network - complete graph
network FastMISNetwork
{
    parameters:
        int numNodes = default(6);
        @display("bgb=600,600");

    submodules:
        node[numNodes]: FastMISNode {
            parameters:
                nodeId = index;
                @display("p=300+200*cos(2*3.14159*index/6),300+200*sin(2*3.14159*index/6)");
            gates:
                in[sizeof(parent.node)-1];
                out[sizeof(parent.node)-1];
        }

    connections:
        // Complete graph - every node connected to every other node
        for i=0..sizeof(node)-1, for j=0..sizeof(node)-1, if i!=j {
            node[i].out[j<i ? j : j-1] --> node[j].in[i<j ? i : i-1];
        }
}

// Fast MIS network - grid topology
network FastMISGridNetwork
{
    parameters:
        int rows = default(3);
        int cols = default(3);
        @display("bgb=600,600");

    submodules:
        node[rows*cols]: FastMISNode {
            parameters:
                nodeId = index;
                @display("p=100+150*(index%3),100+150*(index/3)");
            gates:
                in[4];  // up, down, left, right
                out[4]; // up, down, left, right
        }

    connections allowunconnected:
        // Grid connections
        for i=0..rows-1, for j=0..cols-1 {
            // Right connection
            node[i*cols+j].out[0] --> node[i*cols+((j+1)%cols)].in[2] if j < cols-1;
            // Down connection  
            node[i*cols+j].out[1] --> node[((i+1)%rows)*cols+j].in[3] if i < rows-1;
            // Left connection
            node[i*cols+j].out[2] --> node[i*cols+((j-1+cols)%cols)].in[0] if j > 0;
            // Up connection
            node[i*cols+j].out[3] --> node[((i-1+rows)%rows)*cols+j].in[1] if i > 0;
        }
}

// Slow MIS network - complete graph
network SlowMISNetwork
{
    parameters:
        int numNodes = default(6);
        @display("bgb=600,600");

    submodules:
        node[numNodes]: SlowMISNode {
            parameters:
                nodeId = index;
                @display("p=300+200*cos(2*3.14159*index/6),300+200*sin(2*3.14159*index/6)");
            gates:
                in[sizeof(parent.node)-1];
                out[sizeof(parent.node)-1];
        }

    connections:
        // Complete graph - every node connected to every other node
        for i=0..sizeof(node)-1, for j=0..sizeof(node)-1, if i!=j {
            node[i].out[j<i ? j : j-1] --> node[j].in[i<j ? i : i-1];
        }
}

// Slow MIS network - grid topology
network SlowMISGridNetwork
{
    parameters:
        int rows = default(3);
        int cols = default(3);
        @display("bgb=600,600");

    submodules:
        node[rows*cols]: SlowMISNode {
            parameters:
                nodeId = index;
                @display("p=100+150*(index%3),100+150*(index/3)");
            gates:
                in[4];  // up, down, left, right
                out[4]; // up, down, left, right
        }

    connections allowunconnected:
        // Grid connections
        for i=0..rows-1, for j=0..cols-1 {
            // Right connection
            node[i*cols+j].out[0] --> node[i*cols+((j+1)%cols)].in[2] if j < cols-1;
            // Down connection  
            node[i*cols+j].out[1] --> node[((i+1)%rows)*cols+j].in[3] if i < rows-1;
            // Left connection
            node[i*cols+j].out[2] --> node[i*cols+((j-1+cols)%cols)].in[0] if j > 0;
            // Up connection
            node[i*cols+j].out[3] --> node[((i-1+rows)%rows)*cols+j].in[1] if i > 0;
        }
}

// Fast MIS network - random graph (Erdős-Rényi)
network FastMISRandomNetwork
{
    parameters:
        int numNodes = default(10);
        double edgeProbability = default(0.3); // Probability that any two nodes are connected
        @display("bgb=800,800");

    submodules:
        node[numNodes]: FastMISNode {
            parameters:
                nodeId = index;
                @display("p=400+300*cos(2*3.14159*index/numNodes),400+300*sin(2*3.14159*index/numNodes)");
            gates:
                in[sizeof(parent.node)-1];
                out[sizeof(parent.node)-1];
        }

    connections allowunconnected:
        // Random graph - each pair of nodes connected with probability edgeProbability
        // Both directions created together to ensure undirected graph
        for i=0..sizeof(node)-1, for j=i+1..sizeof(node)-1, if uniform(0,1) < edgeProbability {
            node[i].out[j-1] --> node[j].in[i];
            node[j].out[i] --> node[i].in[j-1];
        }
}

// Slow MIS network - random graph (Erdős-Rényi)
network SlowMISRandomNetwork
{
    parameters:
        int numNodes = default(10);
        double edgeProbability = default(0.3); // Probability that any two nodes are connected
        @display("bgb=800,800");

    submodules:
        node[numNodes]: SlowMISNode {
            parameters:
                nodeId = index;
                @display("p=400+300*cos(2*3.14159*index/numNodes),400+300*sin(2*3.14159*index/numNodes)");
            gates:
                in[sizeof(parent.node)-1];
                out[sizeof(parent.node)-1];
        }

    connections allowunconnected:
        // Random graph - each pair of nodes connected with probability edgeProbability
        // Both directions created together to ensure undirected graph
        for i=0..sizeof(node)-1, for j=i+1..sizeof(node)-1, if uniform(0,1) < edgeProbability {
            node[i].out[j-1] --> node[j].in[i];
            node[j].out[i] --> node[i].in[j-1];
        }
}

// Desire-Level MIS network - complete graph
network DesireLevelMISNetwork
{
    parameters:
        int numNodes = default(6);
        @display("bgb=600,600");

    submodules:
        node[numNodes]: DesireLevelMISNode {
            parameters:
                nodeId = index;
                @display("p=300+200*cos(2*3.14159*index/6),300+200*sin(2*3.14159*index/6)");
            gates:
                in[sizeof(parent.node)-1];
                out[sizeof(parent.node)-1];
        }

    connections:
        // Complete graph - every node connected to every other node
        for i=0..sizeof(node)-1, for j=0..sizeof(node)-1, if i!=j {
            node[i].out[j<i ? j : j-1] --> node[j].in[i<j ? i : i-1];
        }
}

// Desire-Level MIS network - grid topology
network DesireLevelMISGridNetwork
{
    parameters:
        int rows = default(3);
        int cols = default(3);
        @display("bgb=600,600");

    submodules:
        node[rows*cols]: DesireLevelMISNode {
            parameters:
                nodeId = index;
                @display("p=100+150*(index%3),100+150*(index/3)");
            gates:
                in[4];  // up, down, left, right
                out[4]; // up, down, left, right
        }

    connections allowunconnected:
        // Grid connections
        for i=0..rows-1, for j=0..cols-1 {
            // Right connection
            node[i*cols+j].out[0] --> node[i*cols+((j+1)%cols)].in[2] if j < cols-1;
            // Down connection  
            node[i*cols+j].out[1] --> node[((i+1)%rows)*cols+j].in[3] if i < rows-1;
            // Left connection
            node[i*cols+j].out[2] --> node[i*cols+((j-1+cols)%cols)].in[0] if j > 0;
            // Up connection
            node[i*cols+j].out[3] --> node[((i-1+rows)%rows)*cols+j].in[1] if i > 0;
        }
}

// Desire-Level MIS network - random graph (Erdős-Rényi)
network DesireLevelMISRandomNetwork
{
    parameters:
        int numNodes = default(10);
        double edgeProbability = default(0.3); // Probability that any two nodes are connected
        @display("bgb=800,800");

    submodules:
        node[numNodes]: DesireLevelMISNode {
            parameters:
                nodeId = index;
                @display("p=400+300*cos(2*3.14159*index/numNodes),400+300*sin(2*3.14159*index/numNodes)");
            gates:
                in[sizeof(parent.node)-1];
                out[sizeof(parent.node)-1];
        }

    connections allowunconnected:
        // Random graph - each pair of nodes connected with probability edgeProbability
        // Both directions created together to ensure undirected graph
        for i=0..sizeof(node)-1, for j=i+1..sizeof(node)-1, if uniform(0,1) < edgeProbability {
            node[i].out[j-1] --> node[j].in[i];
            node[j].out[i] --> node[i].in[j-1];
        }
}