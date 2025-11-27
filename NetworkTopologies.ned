//
// Simple Ring Network Topology
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