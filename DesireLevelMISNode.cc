#include "DesireLevelMISNode.h"
#include <iostream>
#include <algorithm>

Define_Module(DesireLevelMISNode);

void DesireLevelMISNode::initialize() {
    nodeId = par("nodeId");
    currentRound = 0;
    inMIS = false;
    terminated = false;
    isMarked = false;
    desireLevel = 0.5;  // Initial desire level as per algorithm
    effectiveDegree = 0.0;
    numRoundsUntilTermination = 0;
    
    // Initialize timing parameters
    roundInterval = par("roundInterval").doubleValue();
    initialStartDelay = par("initialStartDelay").doubleValue();
    desireLevelSendDelay = par("desireLevelSendDelay").doubleValue();
    
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
    roundStartMsg = new cMessage("roundStart");
    sendDesireLevelMsg = new cMessage("sendDesireLevel");
    checkMarkingMsg = new cMessage("checkMarking");
    
    // Set default visual appearance for active nodes
    getDisplayString().setTagArg("i", 0, "device/laptop");
    getDisplayString().setTagArg("i", 1, "blue");
    getDisplayString().setTagArg("i", 2, "35");
    
    // Start the algorithm with uniform delay
    scheduleAt(simTime() + uniform(0, initialStartDelay), roundStartMsg);
    
    EV << "DesireLevelMISNode " << nodeId << " initialized with " 
       << activeNeighbors.size() << " neighbors, initial desire level = " 
       << desireLevel << endl;
}

void DesireLevelMISNode::handleMessage(cMessage *msg) {
    if (terminated && !msg->isSelfMessage()) {
        delete msg;
        return;
    }

    // Process self messages
    if (msg == roundStartMsg) {
        startNewRound();
        // Print active neighbors of node nodeID
        EV << "Node " << nodeId << " active neighbors: ";
        for (auto it = activeNeighbors.begin(); it != activeNeighbors.end(); ++it) {
            if (it != activeNeighbors.begin()) EV << ", ";
            EV << *it;
        }
        EV << " (total: " << activeNeighbors.size() << ")" << endl;
        if (activeNeighbors.size() == 0) {
            joinMIS();
        }
    } else if (msg == sendDesireLevelMsg) {
        sendDesireLevel();
    } else if (msg == checkMarkingMsg) {
        checkAndJoinMIS();
    } else {
        // Process messages from others
        if (MISDesireLevelMessage *desireMsg = dynamic_cast<MISDesireLevelMessage*>(msg)) {
            processDesireLevelMessage(desireMsg);
        } else if (MISMarkMessage *markMsg = dynamic_cast<MISMarkMessage*>(msg)) {
            processMarkMessage(markMsg);
        } else if (MISJoinNotification *joinMsg = dynamic_cast<MISJoinNotification*>(msg)) {
            processJoinNotification(joinMsg);
        } else if (MISTerminateNotification *termMsg = dynamic_cast<MISTerminateNotification*>(msg)) {
            processTerminateNotification(termMsg);
        } else {
            EV_WARN << "Unknown message type received!" << endl;
        }
        delete msg;
    }
}

void DesireLevelMISNode::startNewRound() {
    if (terminated) return;
    
    currentRound++;
    numRoundsUntilTermination++;
    resetRoundData();
    
    EV << "Node " << nodeId << " starting round " << currentRound 
       << " with desire level " << desireLevel 
       << " and effective degree " << effectiveDegree << endl;
    
    // Update desire level based on effective degree from previous round
    updateDesireLevel();
    
    // Schedule sending desire level after configured delay
    scheduleAt(simTime() + desireLevelSendDelay, sendDesireLevelMsg);
    
    // Schedule next round
    scheduleAt(simTime() + roundInterval, roundStartMsg);
}

void DesireLevelMISNode::sendDesireLevel() {
    if (terminated) return;
    
    EV << "Node " << nodeId << " sending desire level: " << desireLevel << endl;
    
    // Send desire level to all active neighbors
    MISDesireLevelMessage *msg = new MISDesireLevelMessage("DesireLevel");
    msg->setSenderId(nodeId);
    msg->setDesireLevel(desireLevel);
    msg->setRound(currentRound);
    
    broadcastToNeighbors(msg);
    
    // After sending desire level, schedule marking
    // Give time for all nodes to exchange desire levels
    scheduleAt(simTime() + desireLevelSendDelay, checkMarkingMsg);
}

void DesireLevelMISNode::checkAndJoinMIS() {
    if (terminated) return;
    
    // Perform marking with probability equal to desire level
    performMarking();
    
    // Send mark status to neighbors
    MISMarkMessage *msg = new MISMarkMessage("Mark");
    msg->setSenderId(nodeId);
    msg->setIsMarked(isMarked);
    msg->setRound(currentRound);
    
    broadcastToNeighbors(msg);
    
    EV << "Node " << nodeId << " marked: " << (isMarked ? "YES" : "NO") 
       << " with probability " << desireLevel << endl;
}

void DesireLevelMISNode::performMarking() {
    // Mark with probability equal to desire level
    isMarked = (uniform(0, 1) < desireLevel);
}

void DesireLevelMISNode::updateDesireLevel() {
    double oldDesireLevel = desireLevel;
    
    if (effectiveDegree >= 2.0) {
        // Decrease desire: p_{t+1}(v) = p_t(v) / 2
        desireLevel = desireLevel / 2.0;
    } else {
        // Increase desire: p_{t+1}(v) = min{2 * p_t(v), 0.5}
        desireLevel = std::min(2.0 * desireLevel, 0.5);
    }
    
    EV << "Node " << nodeId << " updated desire level from " << oldDesireLevel 
       << " to " << desireLevel << " (effective degree: " << effectiveDegree << ")" << endl;
}

double DesireLevelMISNode::calculateEffectiveDegree() {
    double sum = 0.0;
    for (const auto& pair : neighborDesireLevels) {
        sum += pair.second;
    }
    return sum;
}

void DesireLevelMISNode::joinMIS() {
    inMIS = true;
    
    // Change visual appearance to indicate MIS membership
    getDisplayString().setTagArg("i", 0, "device/server");
    getDisplayString().setTagArg("i", 1, "green");
    getDisplayString().setTagArg("i", 2, "50");
    
    EV << "*** Node " << nodeId << " JOINS MIS in round " << currentRound 
       << " (desire level: " << desireLevel << ") ***" << endl;
    
    // Notify all neighbors
    MISJoinNotification *msg = new MISJoinNotification("JoinMIS");
    msg->setSenderId(nodeId);
    msg->setPhase(currentRound);
    
    broadcastToNeighbors(msg);
    
    // Terminate after joining MIS
    terminate();
}

void DesireLevelMISNode::terminate() {
    if (terminated) return;
    
    terminated = true;
    
    // Change visual appearance for terminated nodes
    if (!inMIS) {
        getDisplayString().setTagArg("i", 0, "device/pc");
        getDisplayString().setTagArg("i", 1, "red");
        getDisplayString().setTagArg("i", 2, "30");
    }
    
    EV << "Node " << nodeId << " TERMINATED in round " << currentRound 
       << (inMIS ? " (IN MIS)" : " (neighbor in MIS)") 
       << " after " << numRoundsUntilTermination << " rounds" << endl;
    
    // Notify neighbors about termination
    MISTerminateNotification *msg = new MISTerminateNotification("Terminate");
    msg->setSenderId(nodeId);
    msg->setPhase(currentRound);
    
    broadcastToNeighbors(msg);
    
    // Cancel all pending messages
    if (roundStartMsg->isScheduled()) cancelEvent(roundStartMsg);
    if (sendDesireLevelMsg->isScheduled()) cancelEvent(sendDesireLevelMsg);
    if (checkMarkingMsg->isScheduled()) cancelEvent(checkMarkingMsg);
}

void DesireLevelMISNode::processDesireLevelMessage(MISDesireLevelMessage *msg) {
    if (msg->getRound() != currentRound) {
        EV_WARN << "Node " << nodeId << " received desire level from round " 
                << msg->getRound() << " but current round is " << currentRound << endl;
        return;
    }
    
    int senderId = msg->getSenderId();
    double senderDesireLevel = msg->getDesireLevel();
    
    // Only accept from active neighbors
    if (activeNeighbors.find(senderId) != activeNeighbors.end()) {
        neighborDesireLevels[senderId] = senderDesireLevel;
        
        // Recalculate effective degree
        effectiveDegree = calculateEffectiveDegree();
        
        EV << "Node " << nodeId << " received desire level " << senderDesireLevel 
           << " from neighbor " << senderId 
           << ", effective degree now: " << effectiveDegree << endl;
    }
}

void DesireLevelMISNode::processMarkMessage(MISMarkMessage *msg) {
    if (msg->getRound() != currentRound) {
        EV_WARN << "Node " << nodeId << " received mark from round " 
                << msg->getRound() << " but current round is " << currentRound << endl;
        return;
    }
    
    int senderId = msg->getSenderId();
    bool senderMarked = msg->isMarked();
    
    // Only accept from active neighbors
    if (activeNeighbors.find(senderId) != activeNeighbors.end()) {
        if (senderMarked) {
            markedNeighbors.insert(senderId);
            
            EV << "Node " << nodeId << " is notified that neighbor " << senderId 
               << " is marked in round " << currentRound << endl;
        }
        
        // Check if we have received marks from all neighbors
        // If we are marked and no neighbor is marked, join MIS
        if (isMarked && markedNeighbors.empty()) {
            // Check if we've received mark status from all neighbors
            // This is a simplification - in practice we might need to wait
            // We'll join if no marked neighbors have been reported yet
            EV << "Node " << nodeId << " is marked and no neighbors are marked, attempting to join MIS" << endl;
            joinMIS();
        }
    }
}

void DesireLevelMISNode::processJoinNotification(MISJoinNotification *msg) {
    int senderId = msg->getSenderId();
    
    if (activeNeighbors.find(senderId) != activeNeighbors.end()) {
        EV << "Node " << nodeId << " is notified that neighbor " << senderId 
           << " joined MIS" << endl;
        
        // Remove from active neighbors
        activeNeighbors.erase(senderId);
        neighborDesireLevels.erase(senderId);
        
        // Recalculate effective degree
        effectiveDegree = calculateEffectiveDegree();
        
        // Terminate because a neighbor joined MIS
        terminate();
    }
}

void DesireLevelMISNode::processTerminateNotification(MISTerminateNotification *msg) {
    int senderId = msg->getSenderId();
    
    // Remove terminated neighbor from active set
    if (activeNeighbors.find(senderId) != activeNeighbors.end()) {
        activeNeighbors.erase(senderId);
        neighborDesireLevels.erase(senderId);
        
        // Recalculate effective degree
        effectiveDegree = calculateEffectiveDegree();
        
        EV << "Node " << nodeId << " is notified that neighbor " << senderId 
           << " terminated. Active neighbors: " << activeNeighbors.size() << endl;
    }
}

void DesireLevelMISNode::broadcastToNeighbors(cMessage *msg) {
    for (int i = 0; i < gateSize("out"); i++) {
        if (gate("out", i)->isConnected()) {
            send(msg->dup(), "out", i);
        }
    }
    delete msg;
}

void DesireLevelMISNode::resetRoundData() {
    isMarked = false;
    markedNeighbors.clear();
    // Note: We keep neighborDesireLevels as they carry over between rounds
}

void DesireLevelMISNode::finish() {
    cancelAndDelete(roundStartMsg);
    cancelAndDelete(sendDesireLevelMsg);
    cancelAndDelete(checkMarkingMsg);
    
    // Print to both EV and cout to ensure visibility
    std::string msg = "DesireLevelMIS Node " + std::to_string(nodeId) + 
                      " finished in " + std::to_string(numRoundsUntilTermination) + 
                      " rounds. " + (inMIS ? "IN MIS" : "NOT in MIS") +
                      " (final desire level: " + std::to_string(desireLevel) + ")";
    
    EV << msg << endl;
    std::cout << "INFO: " << msg << std::endl;
    
    // Record statistics
    recordScalar("rounds", numRoundsUntilTermination);
    recordScalar("inMIS", inMIS ? 1 : 0);
    recordScalar("finalDesireLevel", desireLevel);
}
