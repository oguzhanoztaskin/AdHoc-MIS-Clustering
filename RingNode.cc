#include "RingNode.h"

Define_Module(RingNode);

void RingNode::initialize() {
    nodeId = par("nodeId");
    messageCount = 0;
    selfMsg = nullptr;
    
    // Schedule initial message for node 0
    if (nodeId == 0) {
        selfMsg = new cMessage("start");
        scheduleAt(simTime() + 1.0, selfMsg);
    }
    
    EV << "RingNode " << nodeId << " initialized" << endl;
}

void RingNode::handleMessage(cMessage *msg) {
    if (msg == selfMsg) {
        // Node 0 starts the ring protocol by sending a message
        if (strcmp(msg->getName(), "start") == 0) {
            EV << "Node " << nodeId << " starting ring protocol" << endl;
            
            RingMessage *ringMsg = new RingMessage("Token");
            ringMsg->setSenderId(nodeId);
            ringMsg->setContent("Hello from ring!");
            ringMsg->setHopCount(1);
            
            send(ringMsg, "out", 0);  // Send to next node in ring
        }
    } else {
        // Handle received ring message
        RingMessage *ringMsg = check_and_cast<RingMessage*>(msg);
        
        EV << "Node " << nodeId << " received message from node " << ringMsg->getSenderId() 
           << " with content: " << ringMsg->getContent() 
           << " (hops: " << ringMsg->getHopCount() << ")" << endl;
        
        messageCount++;
        
        // If message completed the ring (back to sender), don't forward
        if (ringMsg->getSenderId() == nodeId) {
            EV << "Node " << nodeId << " - Token completed the ring!" << endl;
        } else {
            // Forward message to next node
            ringMsg->setHopCount(ringMsg->getHopCount() + 1);
            send(ringMsg, "out", 0);
            return; // Don't delete the message, it's forwarded
        }
        
        delete msg;
    }
}

void RingNode::finish() {
    cancelAndDelete(selfMsg);
    EV << "Node " << nodeId << " finished. Processed " << messageCount << " messages." << endl;
}