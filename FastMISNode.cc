#include "FastMISNode.h"
#include <iostream>

Define_Module(FastMISNode);

void FastMISNode::initialize() {
    nodeId = par("nodeId");
    currentPhase = 1;
    inMIS = false;
    terminated = false;
    myRandomValue = 0.0;
    finalRandomValue = 0.0;
    totalPhases = 0;
    
    // Initialize timing parameters
    phaseInterval = par("phaseInterval").doubleValue();
    randomValueTimeout = par("randomValueTimeout").doubleValue();
    decisionTimeout = par("decisionTimeout").doubleValue();
    
    // Initialize neighbor set based on connected gates
    for (int i = 0; i < gateSize("out"); i++) {
        if (gate("out", i)->isConnected()) {
            cGate *connectedGate = gate("out", i)->getNextGate();
            if (connectedGate && connectedGate->getOwnerModule()) {
                int neighborId = connectedGate->getOwnerModule()->par("nodeId");
                activeNeighbors.insert(neighborId);
            }
        }
    }
    
    // Initialize self-messages
    phaseStartMsg = new cMessage("phaseStart");
    randomValueTimeoutMsg = new cMessage("randomValueTimeout");
    decisionTimeoutMsg = new cMessage("decisionTimeout");
    
    // Set default visual appearance for active nodes
    getDisplayString().setTagArg("i", 0, "device/laptop");
    getDisplayString().setTagArg("i", 1, "blue");
    getDisplayString().setTagArg("i", 2, "35");
    
    // Start the algorithm
    scheduleAt(simTime() + uniform(0, 1), phaseStartMsg);
    
    EV << "FastMISNode " << nodeId << " initialized with " << activeNeighbors.size() << " neighbors" << endl;
}

void FastMISNode::handleMessage(cMessage *msg) {
    if (terminated) {
        delete msg;
        return;
    }

    // Process self messages
    if (msg == phaseStartMsg) {
        startNewPhase();
    } else if (msg == randomValueTimeoutMsg) {
        makeDecision();
    } else if (msg == decisionTimeoutMsg) {
        // Start next phase if not terminated
        if (!terminated) {
            scheduleAt(simTime() + phaseInterval, phaseStartMsg);
        }
    } else {
        // Process messages from others
        // We need to delete these at the end
        if (MISRandomValue *randMsg = dynamic_cast<MISRandomValue*>(msg)) {
            processRandomValue(randMsg);
        } else if (MISJoinNotification *joinMsg = dynamic_cast<MISJoinNotification*>(msg)) {
            processJoinNotification(joinMsg);
        } else if (MISTerminateNotification *termMsg = dynamic_cast<MISTerminateNotification*>(msg)) {
            processTerminateNotification(termMsg);
        } else {
            // Unknown message, ignore
        }
        delete msg;
    }
}

void FastMISNode::startNewPhase() {
    if (terminated) return;
    
    totalPhases++;
    currentPhase++;
    resetPhaseData();
    
    EV << "Node " << nodeId << " starting phase " << currentPhase << endl;
    
    sendRandomValue();
    
    // Schedule timeout for decision making
    scheduleAt(simTime() + randomValueTimeout, randomValueTimeoutMsg);
}

void FastMISNode::sendRandomValue() {
    // Generate random value
    myRandomValue = uniform(0, 1);
    
    EV << "Node " << nodeId << " generated random value: " << myRandomValue << endl;
    
    // Send to all active neighbors
    MISRandomValue *msg = new MISRandomValue("RandomValue");
    msg->setSenderId(nodeId);
    msg->setRandomValue(myRandomValue);
    msg->setPhase(currentPhase);
    
    broadcastToNeighbors(msg);
}

void FastMISNode::makeDecision() {
    if (terminated) return;
    
    // Store the random value from this deciding phase
    finalRandomValue = myRandomValue;
    
    EV << "Node " << nodeId << " making decision in phase " << currentPhase << endl;
    EV << "My random value: " << myRandomValue << ", received " << neighborRandomValues.size() 
       << " neighbor values" << endl;
    
    if (shouldJoinMIS()) {
        joinMIS();
    } else {
        // Schedule next phase
        scheduleAt(simTime() + decisionTimeout, decisionTimeoutMsg);
    }
}

bool FastMISNode::shouldJoinMIS() {
    // Check if my random value is smaller than all neighbors' values
    for (const auto& pair : neighborRandomValues) {
        if (myRandomValue >= pair.second) {
            EV << "Node " << nodeId << " - my value " << myRandomValue 
               << " >= neighbor " << pair.first << " value " << pair.second << endl;
            return false;
        }
    }
    
    // Also need to have received values from all active neighbors
    for (int neighborId : activeNeighbors) {
        if (neighborRandomValues.find(neighborId) == neighborRandomValues.end()) {
            EV << "Node " << nodeId << " - missing value from neighbor " << neighborId << endl;
            return false;
        }
    }
    
    return !activeNeighbors.empty(); // Only join if we have neighbors
}

void FastMISNode::joinMIS() {
    inMIS = true;
    
    // Change visual appearance to indicate MIS membership
    getDisplayString().setTagArg("i", 0, "device/server");
    getDisplayString().setTagArg("i", 1, "green");
    getDisplayString().setTagArg("i", 2, "50");
    
    EV << "*** Node " << nodeId << " JOINS MIS in phase " << currentPhase << " ***" << endl;
    
    // Notify all neighbors
    MISJoinNotification *msg = new MISJoinNotification("JoinMIS");
    msg->setSenderId(nodeId);
    msg->setPhase(currentPhase);
    
    broadcastToNeighbors(msg);
    
    // Terminate after joining MIS
    scheduleAt(simTime() + 0.1, decisionTimeoutMsg);
    terminate();
}

void FastMISNode::terminate() {
    if (terminated) return;
    
    terminated = true;
    
    // Change visual appearance for terminated nodes
    if (!inMIS) {
        getDisplayString().setTagArg("i", 0, "device/pc");
        getDisplayString().setTagArg("i", 1, "red");
        getDisplayString().setTagArg("i", 2, "30");
    }
    
    EV << "Node " << nodeId << " TERMINATED in phase " << currentPhase 
       << (inMIS ? " (IN MIS)" : " (neighbor in MIS)") << endl;
    
    // Notify neighbors about termination
    MISTerminateNotification *msg = new MISTerminateNotification("Terminate");
    msg->setSenderId(nodeId);
    msg->setPhase(currentPhase);
    
    broadcastToNeighbors(msg);
    
    // Cancel all pending messages
    if (phaseStartMsg->isScheduled()) cancelEvent(phaseStartMsg);
    if (randomValueTimeoutMsg->isScheduled()) cancelEvent(randomValueTimeoutMsg);
    if (decisionTimeoutMsg->isScheduled()) cancelEvent(decisionTimeoutMsg);
}

void FastMISNode::processRandomValue(MISRandomValue *msg) {
    if (terminated || msg->getPhase() != currentPhase) return;
    
    int senderId = msg->getSenderId();
    double value = msg->getRandomValue();
    
    // Only accept from active neighbors
    if (activeNeighbors.find(senderId) != activeNeighbors.end()) {
        neighborRandomValues[senderId] = value;
        
        EV << "Node " << nodeId << " received random value " << value 
           << " from neighbor " << senderId << endl;
    }
}

void FastMISNode::processJoinNotification(MISJoinNotification *msg) {
    if (terminated || msg->getPhase() != currentPhase) return;
    
    int senderId = msg->getSenderId();
    
    if (activeNeighbors.find(senderId) != activeNeighbors.end()) {
        neighborsInMIS.insert(senderId);
        
        EV << "Node " << nodeId << " notified that neighbor " << senderId 
           << " joined MIS" << endl;
        
        // Terminate because a neighbor joined MIS
        terminate();
    }
}

void FastMISNode::processTerminateNotification(MISTerminateNotification *msg) {
    if (terminated) return;
    
    int senderId = msg->getSenderId();
    
    // Remove terminated neighbor from active set
    activeNeighbors.erase(senderId);
    neighborRandomValues.erase(senderId);
    
    EV << "Node " << nodeId << " notified that neighbor " << senderId 
       << " terminated. Active neighbors: " << activeNeighbors.size() << endl;
    
    // TODO: Do not MIS neighbors terminate too? Would not termination message and MIS join message collide?
    // If no active neighbors left, might need to join MIS
    if (activeNeighbors.empty() && !inMIS && !terminated) {
        EV << "Node " << nodeId << " has no active neighbors left - joining MIS" << endl;
        inMIS = true;
        // Change visual appearance for isolated nodes joining MIS
        getDisplayString().setTagArg("i", 0, "device/server");
        getDisplayString().setTagArg("i", 1, "green");
        getDisplayString().setTagArg("i", 2, "50");
        terminate();
    }
}

void FastMISNode::broadcastToNeighbors(cMessage *msg) {
    for (int i = 0; i < gateSize("out"); i++) {
        if (gate("out", i)->isConnected()) {
            send(msg->dup(), "out", i);
        }
    }
    delete msg;
}

void FastMISNode::resetPhaseData() {
    neighborRandomValues.clear();
    neighborsInMIS.clear();
    myRandomValue = 0.0;
}

void FastMISNode::finish() {
    cancelAndDelete(phaseStartMsg);
    cancelAndDelete(randomValueTimeoutMsg);
    cancelAndDelete(decisionTimeoutMsg);
    
    // Print to both EV and cout to ensure visibility
    std::string msg = "Node " + std::to_string(nodeId) + " finished after " + 
                      std::to_string(totalPhases) + " phases. " +
                      (inMIS ? "IN MIS" : "NOT in MIS") + 
                      " (final random value: " + std::to_string(finalRandomValue) + ")";
    
    EV << msg << endl;
    std::cout << "INFO: " << msg << std::endl;
    
    // Record statistics
    recordScalar("phases", totalPhases);
    recordScalar("inMIS", inMIS ? 1 : 0);
    recordScalar("finalRandomValue", finalRandomValue);
}