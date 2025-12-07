#include "SlowMISNode.h"
#include <iostream>

Define_Module(SlowMISNode);

void SlowMISNode::initialize() {
    nodeId = par("nodeId");
    inMIS = false;
    terminated = false;
    
    // Initialize timing parameters
    initialStartDelay = par("initialStartDelay").doubleValue();
    
    // Initialize self-messages
    startAlgorithmMsg = new cMessage("startAlgorithm");
    
    // Initialize neighbor set based on connected gates
    for (int i = 0; i < gateSize("out"); i++) {
        if (gate("out", i)->isConnected()) {
            cGate *connectedGate = gate("out", i)->getNextGate();
            if (connectedGate && connectedGate->getOwnerModule()) {
                int neighborId = connectedGate->getOwnerModule()->par("nodeId");
                neighbors.insert(neighborId);
            }
        }
    }

    // Set default visual appearance for active nodes
    getDisplayString().setTagArg("i", 0, "device/laptop");
    getDisplayString().setTagArg("i", 1, "blue");
    getDisplayString().setTagArg("i", 2, "35");
    
    // Start the algorithm with uniform delay
    scheduleAt(simTime() + uniform(0, initialStartDelay), startAlgorithmMsg);
    
    EV << "SlowMISNode " << nodeId << " initialized" << endl;
}

void SlowMISNode::handleMessage(cMessage *msg) {
    if (terminated && !msg->isSelfMessage()) {
        // Delete messages from others that are sent after we terminated.
        delete msg;
        return;
    }

    // Process self messages
    if (msg == startAlgorithmMsg) {
        // Boot start - the node with the highest id joins MIS
        // and its neighbors terminate.
        tryMakeDecision();
        return;
    } else {
        // Process messages from others
        if (MISJoinNotification *joinMsg = dynamic_cast<MISJoinNotification*>(msg)) {
            processJoinNotification(joinMsg);
        } else if (MISTerminateNotification *termMsg = dynamic_cast<MISTerminateNotification*>(msg)) {
            processTerminateNotification(termMsg);
        }
        delete msg;
    }
}

void SlowMISNode::tryMakeDecision() {
    EV << "Node " << nodeId << " checking decision condition..." << endl;
    
    Decision decision = makeDecision();
    
    switch (decision) {
        case JOIN_MIS:
            joinMIS();
            break;
        case TERMINATE:
            terminate();
            break;
        case NO_DECISION:
            EV << "Node " << nodeId << " cannot make a decision yet" << endl;
            break;
    }
}

SlowMISNode::Decision SlowMISNode::makeDecision() {
    // Check all higher-ID neighbors
    for (int neighborId : neighbors) {
        if (neighborId > nodeId) {
            if (neighborDecisions.find(neighborId) == neighborDecisions.end()) {
                // Neighbor has not yet notified us, so we cannot decide anything yet.
                EV << "Node " << nodeId << " cannot make a decision as not all higher neighbors made a decision." << endl;
                return NO_DECISION;
            } else {
                if (neighborDecisions[neighborId]) {
                    // Higher neighbor joined MIS, thus we cannot and must terminate.
                    EV << "Node " << nodeId << " must terminate as a higher-up joined MIS." << endl;
                    return TERMINATE;
                } else {
                    // Neighbor terminated without joining, continue checking.
                }
            }
        }
    }
    
    // All higher-ID neighbors have decided not to join MIS, so we can join
    EV << "Node " << nodeId << " - all higher-ID neighbors decided not to join MIS" << endl;
    return JOIN_MIS;
}

void SlowMISNode::joinMIS() {
    inMIS = true;
    
    // Change visual appearance to indicate MIS membership
    getDisplayString().setTagArg("i", 0, "device/server");
    getDisplayString().setTagArg("i", 1, "green");
    getDisplayString().setTagArg("i", 2, "50");
    
    EV << "*** Node " << nodeId << " JOINS MIS ***" << endl;
    
    // Notify all neighbors
    MISJoinNotification *msg = new MISJoinNotification("JoinMIS");
    msg->setSenderId(nodeId);
    msg->setPhase(0); // Not really used in slow MIS
    
    broadcastToLowerNeighbors(msg);
    
    terminate();
}

void SlowMISNode::terminate() {
    if (terminated) return;
    
    terminated = true;
    
    // Change visual appearance for terminated nodes
    if (!inMIS) {
        getDisplayString().setTagArg("i", 0, "device/pc");
        getDisplayString().setTagArg("i", 1, "red");
        getDisplayString().setTagArg("i", 2, "30");
        
        // If not in MIS, notify neighbors that we decided not to join
        MISTerminateNotification *msg = new MISTerminateNotification("NotJoining");
        msg->setSenderId(nodeId);
        msg->setPhase(0); // Not really used in slow MIS
        
        broadcastToLowerNeighbors(msg);
    }
    
    EV << "Node " << nodeId << " TERMINATED " 
       << (inMIS ? " (IN MIS)" : " (not in MIS)") << endl;
}

void SlowMISNode::processJoinNotification(MISJoinNotification *msg) {
    int senderId = msg->getSenderId();
    
    if (neighbors.find(senderId) != neighbors.end()) {
        // Record that this neighbor joined MIS
        neighborDecisions[senderId] = true;
        
        EV << "Node " << nodeId << " is notified that neighbor " << senderId 
           << " joined MIS" << endl;
        
        // If a neighbor joined MIS, we cannot join and should terminate
        terminate();
    }
}

void SlowMISNode::processTerminateNotification(MISTerminateNotification *msg) {
    int senderId = msg->getSenderId();
    
    if (neighbors.find(senderId) != neighbors.end()) {
        // Record that this neighbor decided not to join MIS
        neighborDecisions[senderId] = false;
        
        EV << "Node " << nodeId << " is notified that neighbor " << senderId 
           << " decided not to join MIS" << endl;
        tryMakeDecision();
    }
}

void SlowMISNode::broadcastToNeighbors(cMessage *msg) {
    for (int i = 0; i < gateSize("out"); i++) {
        if (gate("out", i)->isConnected()) {
            send(msg->dup(), "out", i);
        }
    }
    delete msg;
}

void SlowMISNode::broadcastToLowerNeighbors(cMessage *msg) {
    for (int i = 0; i < gateSize("out"); i++) {
        if (gate("out", i)->isConnected()) {
            cGate *connectedGate = gate("out", i)->getNextGate();
            if (connectedGate && connectedGate->getOwnerModule()) {
                int neighborId = connectedGate->getOwnerModule()->par("nodeId");
                if (neighborId < nodeId) {
                    send(msg->dup(), "out", i);
                }
            }
        }
    }
    delete msg;
}

void SlowMISNode::finish() {
    // Cancel and delete self messages
    if (startAlgorithmMsg) {
        cancelAndDelete(startAlgorithmMsg);
    }
    
    // Print to both EV and cout to ensure visibility
    std::string msg = "SlowMIS Node " + std::to_string(nodeId) + " finished. " +
                      (inMIS ? "IN MIS" : "NOT in MIS");
    
    EV << msg << endl;
    std::cout << "INFO: " << msg << std::endl;
    
    // Record statistics
    recordScalar("inMIS", inMIS ? 1 : 0);
}