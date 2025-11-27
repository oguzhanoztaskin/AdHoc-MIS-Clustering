#ifndef SLOWMISNODE_H
#define SLOWMISNODE_H

#include <omnetpp.h>
#include <set>
#include <map>
#include "message_m.h"

using namespace omnetpp;

class SlowMISNode : public cSimpleModule
{
private:
    int nodeId;
    bool inMIS;
    bool terminated;
    bool neighborDiscoveryComplete;
    
    // Neighbors and their status
    std::set<int> allNeighbors;
    std::set<int> higherIdNeighbors;
    std::map<int, bool> neighborDecisions; // true = joined MIS, false = decided not to join
    
    // Timing parameters
    double checkInterval;
    double discoveryTimeout;
    
    // Self-messages
    cMessage *checkDecisionMsg;
    cMessage *neighborDiscoveryMsg;
    cMessage *discoveryTimeoutMsg;
    
    // Methods
    void startNeighborDiscovery();
    void finishNeighborDiscovery();
    void checkAndMakeDecision();
    bool canJoinMIS();
    void joinMIS();
    void terminate();
    void broadcastToNeighbors(cMessage *msg);
    void processNeighborAnnouncement(cMessage *msg);
    void processJoinNotification(MISJoinNotification *msg);
    void processTerminateNotification(MISTerminateNotification *msg);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif