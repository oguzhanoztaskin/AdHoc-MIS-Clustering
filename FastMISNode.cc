#include "FastMISNode.h"
#include <iostream>

Define_Module(FastMISNode);

void FastMISNode::initialize() {
    nodeId = par("nodeId");
    currentPhase = 1;
    inMIS = false;
    terminated = false;
    myRandomValue = 0.0;
    
    // Initialize timing parameters
    phaseInterval = par("phaseInterval").doubleValue();
    initialStartDelay = par("initialStartDelay").doubleValue();
    randomValueSendDelay = par("randomValueSendDelay").doubleValue();
    
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
    sendRandomValueMsg = new cMessage("sendRandomValue");
    
    // Set default visual appearance for active nodes
    getDisplayString().setTagArg("i", 0, "device/laptop");
    getDisplayString().setTagArg("i", 1, "blue");
    getDisplayString().setTagArg("i", 2, "35");
    
    // Start the algorithm with uniform delay
    scheduleAt(simTime() + uniform(0, initialStartDelay), phaseStartMsg);
    
    EV << "FastMISNode " << nodeId << " initialized with " << activeNeighbors.size() << " neighbors" << endl;
}

void FastMISNode::handleMessage(cMessage *msg) {
    if (terminated && !msg->isSelfMessage()) {
        delete msg;
        return;
    }

    // Process self messages
    if (msg == phaseStartMsg) {
        startNewPhase();
        if (activeNeighbors.size() == 0) {
            // Isolated node, no messages will be received.
            // Try making a decision here.
            tryMakeDecision();
        }
    } else if (msg == sendRandomValueMsg) {
        sendRandomValue();
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
            EV_WARN << "Unknown message has been received!" << endl;
        }
        delete msg;
    }
}

void FastMISNode::startNewPhase() {
    logPhaseEnd();
    
    currentPhase++;
    resetPhaseData();
    
    EV << "Node " << nodeId << " starting phase " << currentPhase << endl;
    
    // Schedule sending random value after configured delay
    scheduleAt(simTime() + randomValueSendDelay, sendRandomValueMsg);

    // Schedule timeout for next phase
    scheduleAt(simTime() + phaseInterval, phaseStartMsg);
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

void FastMISNode::tryMakeDecision() {
    EV << "Node " << nodeId << " making decision in phase " << currentPhase << endl
       << "My random value: " << myRandomValue << ", received " << neighborRandomValues.size() 
       << " neighbor values" << endl;
    
    if (shouldJoinMIS()) {
        JoinMIS();
    }
}

/**
 * Checks if our random value is smallest among all that is received and that
 * if we received a message from every active neighbor.
 */
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
    
    // No neighbor who has not sent its random value
    // No neighbor with greater random value
    // Then we should join MIS
    return true;
}

void FastMISNode::JoinMIS() {
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
    terminate();
}

void FastMISNode::terminate() {
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
}

void FastMISNode::processRandomValue(MISRandomValue *msg) {
    if (msg->getPhase() != currentPhase) {
        EV_WARN << "Node " << nodeId << " received random value " << msg->getRandomValue() 
           << " from neighbor " << msg->getSenderId()
           << " but our phase: " << currentPhase
           << " their phase: " << msg->getPhase() << endl;
        return;
    }
    
    int senderId = msg->getSenderId();
    double value = msg->getRandomValue();
    
    // Only accept from active neighbors
    if (activeNeighbors.find(senderId) != activeNeighbors.end()) {
        neighborRandomValues[senderId] = value;
        
        EV << "Node " << nodeId << " received random value " << value 
           << " from neighbor " << senderId << endl;
    }

    tryMakeDecision();
}

void FastMISNode::processJoinNotification(MISJoinNotification *msg) {
    if (msg->getPhase() != currentPhase) {
        EV_WARN << "Node " << nodeId << " received join notification value from neighbor "
           << msg->getSenderId()
           << " but our phase: " << currentPhase
           << " their phase: " << msg->getPhase() << endl;
        return;
    }
    
    int senderId = msg->getSenderId();
    
    if (activeNeighbors.find(senderId) != activeNeighbors.end()) {
        neighborsInMIS.insert(senderId);
        
        EV << "Node " << nodeId << " is notified that neighbor " << senderId 
           << " joined MIS" << endl;
        
        // Terminate because a neighbor joined MIS
        terminate();
    }
}

void FastMISNode::processTerminateNotification(MISTerminateNotification *msg) {
    int senderId = msg->getSenderId();
    
    // Remove terminated neighbor from active set
    activeNeighbors.erase(senderId);
    neighborRandomValues.erase(senderId);
    
    EV << "Node " << nodeId << " is notified that neighbor " << senderId 
       << " terminated. Active neighbors: " << activeNeighbors.size() << endl;

    // Active neighbors is updated, try joining again
    tryMakeDecision();
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
    myRandomValue = 0.0;
}

void FastMISNode::logPhaseEnd() {
    // Collect all random values (own + neighbors) and sort them
    std::vector<std::pair<double, int>> allValues;
    allValues.push_back({myRandomValue, nodeId}); // Add own value
    for (const auto& pair : neighborRandomValues) {
        allValues.push_back({pair.second, pair.first});
    }
    std::sort(allValues.begin(), allValues.end());
    
    // Build string representation with own value highlighted
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < allValues.size(); ++i) {
        if (i > 0) oss << ", ";
        if (allValues[i].second == nodeId) {
            oss << ">>>" << std::fixed << allValues[i].first << " (SELF)<<<";
        } else {
            oss << std::fixed << allValues[i].first << " (" << allValues[i].second << ")";
        }
    }
    oss << "]";
    
    EV << "Node " << nodeId << " is ending phase " << currentPhase << endl
       << " with its random value " << std::fixed << myRandomValue << endl
       << " with all random values (sorted) " << oss.str() << endl;
}

void FastMISNode::finish() {
    cancelAndDelete(phaseStartMsg);
    cancelAndDelete(sendRandomValueMsg);
    
    // Print to both EV and cout to ensure visibility
    std::string msg = "Node " + std::to_string(nodeId) + " finished at " + 
                      std::to_string(currentPhase) + " phase. " +
                      (inMIS ? "IN MIS" : "NOT in MIS") + 
                      " (final random value: " + std::to_string(myRandomValue) + ")";
    
    EV << msg << endl;
    std::cout << "INFO: " << msg << std::endl;
    
    // Record statistics
    recordScalar("phase", currentPhase);
    recordScalar("inMIS", inMIS ? 1 : 0);
    recordScalar("myRandomValue", myRandomValue);
}