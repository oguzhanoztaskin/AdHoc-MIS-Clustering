#include "SlowMISNode.h"
#include <iostream>

Define_Module(SlowMISNode);

void SlowMISNode::initialize() {
    nodeId = par("nodeId");
    inMIS = false;
    terminated = false;
    neighborDiscoveryComplete = false;
    
    // Initialize timing parameters
    checkInterval = par("checkInterval").doubleValue();
    discoveryTimeout = par("discoveryTimeout").doubleValue();
    
    // Initialize self-messages
    checkDecisionMsg = new cMessage("checkDecision");
    neighborDiscoveryMsg = new cMessage("neighborDiscovery");
    discoveryTimeoutMsg = new cMessage("discoveryTimeout");
    
    // Set default visual appearance for active nodes
    getDisplayString().setTagArg("i", 0, "device/laptop");
    getDisplayString().setTagArg("i", 1, "blue");
    getDisplayString().setTagArg("i", 2, "35");
    
    // Start neighbor discovery
    scheduleAt(simTime() + uniform(0, 0.1), neighborDiscoveryMsg);
    
    EV << "SlowMISNode " << nodeId << " initialized" << endl;
}

void SlowMISNode::startNeighborDiscovery() {
    EV << "Node " << nodeId << " starting neighbor discovery" << endl;
    
    // Broadcast my ID to all neighbors
    MISNeighborAnnouncement *msg = new MISNeighborAnnouncement("NeighborAnnouncement");
    msg->setSenderId(nodeId);
    broadcastToNeighbors(msg);
    
    // Schedule timeout for discovery phase
    scheduleAt(simTime() + discoveryTimeout, discoveryTimeoutMsg);
}

void SlowMISNode::finishNeighborDiscovery() {
    neighborDiscoveryComplete = true;
    
    // Identify neighbors with higher IDs
    for (int neighborId : allNeighbors) {
        if (neighborId > nodeId) {
            higherIdNeighbors.insert(neighborId);
        }
    }
    
    EV << "Node " << nodeId << " discovered " << allNeighbors.size() 
       << " neighbors (" << higherIdNeighbors.size() << " with higher IDs): ";
    for (int id : higherIdNeighbors) {
        EV << id << " ";
    }
    EV << endl;
    
    // Start the MIS algorithm
    scheduleAt(simTime() + uniform(0, checkInterval), checkDecisionMsg);
}

void SlowMISNode::handleMessage(cMessage *msg) {
    if (terminated && msg != neighborDiscoveryMsg && msg != discoveryTimeoutMsg && msg != checkDecisionMsg) {
        delete msg;
        return;
    }

    // Process self messages
    if (msg == neighborDiscoveryMsg) {
        startNeighborDiscovery();
    } else if (msg == discoveryTimeoutMsg) {
        finishNeighborDiscovery();
    } else if (msg == checkDecisionMsg) {
        if (neighborDiscoveryComplete) {
            checkAndMakeDecision();
        }
        
        // Schedule next check if not terminated
        if (!terminated) {
            scheduleAt(simTime() + checkInterval, checkDecisionMsg);
        }
    } else {
        // Process messages from others
        if (MISNeighborAnnouncement *announceMsg = dynamic_cast<MISNeighborAnnouncement*>(msg)) {
            processNeighborAnnouncement(announceMsg);
        } else if (MISJoinNotification *joinMsg = dynamic_cast<MISJoinNotification*>(msg)) {
            processJoinNotification(joinMsg);
        } else if (MISTerminateNotification *termMsg = dynamic_cast<MISTerminateNotification*>(msg)) {
            processTerminateNotification(termMsg);
        }
        delete msg;
    }
}

void SlowMISNode::checkAndMakeDecision() {
    if (terminated) return;
    
    EV << "Node " << nodeId << " checking decision condition..." << endl;
    
    if (canJoinMIS()) {
        joinMIS();
    }
}

bool SlowMISNode::canJoinMIS() {
    // Algorithm 7.3: Join MIS if all neighbors with larger IDs have decided not to join
    
    // Check if we have received decisions from all higher-ID neighbors
    for (int neighborId : higherIdNeighbors) {
        auto it = neighborDecisions.find(neighborId);
        if (it == neighborDecisions.end()) {
            // Haven't received decision from this neighbor yet
            EV << "Node " << nodeId << " - waiting for decision from neighbor " << neighborId << endl;
            return false;
        } else if (it->second == true) {
            // Neighbor joined MIS, so we cannot
            EV << "Node " << nodeId << " - neighbor " << neighborId << " joined MIS" << endl;
            return false;
        }
    }
    
    // All higher-ID neighbors have decided not to join MIS
    EV << "Node " << nodeId << " - all higher-ID neighbors decided not to join MIS" << endl;
    return true;
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
    
    broadcastToNeighbors(msg);
    
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
        
        broadcastToNeighbors(msg);
    }
    
    EV << "Node " << nodeId << " TERMINATED " 
       << (inMIS ? " (IN MIS)" : " (not in MIS)") << endl;
    
    // Cancel pending messages
    if (checkDecisionMsg->isScheduled()) {
        cancelEvent(checkDecisionMsg);
    }
}

void SlowMISNode::processNeighborAnnouncement(cMessage *msg) {
    if (neighborDiscoveryComplete) return;
    
    MISNeighborAnnouncement *announceMsg = dynamic_cast<MISNeighborAnnouncement*>(msg);
    if (!announceMsg) return;
    
    int senderId = announceMsg->getSenderId();
    
    // Add to neighbor set
    allNeighbors.insert(senderId);
    
    EV << "Node " << nodeId << " discovered neighbor " << senderId << endl;
}

void SlowMISNode::processJoinNotification(MISJoinNotification *msg) {
    if (terminated) return;
    
    int senderId = msg->getSenderId();
    
    if (allNeighbors.find(senderId) != allNeighbors.end()) {
        // Record that this neighbor joined MIS
        neighborDecisions[senderId] = true;
        
        EV << "Node " << nodeId << " notified that neighbor " << senderId 
           << " joined MIS" << endl;
        
        // If a neighbor joined MIS, we cannot join and should terminate
        if (!inMIS) {
            terminate();
        }
    }
}

void SlowMISNode::processTerminateNotification(MISTerminateNotification *msg) {
    if (terminated) return;
    
    int senderId = msg->getSenderId();
    
    if (allNeighbors.find(senderId) != allNeighbors.end()) {
        // Record that this neighbor decided not to join MIS
        neighborDecisions[senderId] = false;
        
        EV << "Node " << nodeId << " notified that neighbor " << senderId 
           << " decided not to join MIS" << endl;
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

void SlowMISNode::finish() {
    cancelAndDelete(checkDecisionMsg);
    cancelAndDelete(neighborDiscoveryMsg);
    cancelAndDelete(discoveryTimeoutMsg);
    
    // Print to both EV and cout to ensure visibility
    std::string msg = "SlowMIS Node " + std::to_string(nodeId) + " finished. " +
                      (inMIS ? "IN MIS" : "NOT in MIS");
    
    EV << msg << endl;
    std::cout << "INFO: " << msg << std::endl;
    
    // Record statistics
    recordScalar("inMIS", inMIS ? 1 : 0);
}