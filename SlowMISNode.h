#ifndef SLOWMISNODE_H
#define SLOWMISNODE_H

#include <omnetpp.h>
#include <set>
#include <map>
#include "message_m.h"

using namespace omnetpp;

class SlowMISNode : public cSimpleModule
{
public:
    enum Decision {
        JOIN_MIS,
        TERMINATE,
        NO_DECISION
    };
private:
    int nodeId;
    bool inMIS;
    bool terminated;
    bool neighborDiscoveryComplete;
    
    // Timing parameters
    double initialStartDelay;
    
    // Self messages
    cMessage *startAlgorithmMsg;
    
    // Neighbors and their status
    std::set<int> neighbors;
    std::map<int, bool> neighborDecisions; // true = joined MIS, false = decided not to join
    
    // Methods
    void startNeighborDiscovery();
    void finishNeighborDiscovery();
    void tryMakeDecision();
    Decision makeDecision();
    void joinMIS();
    void terminate();
    void broadcastToNeighbors(cMessage *msg);
    void broadcastToLowerNeighbors(cMessage *msg);
    void processNeighborAnnouncement(cMessage *msg);
    void processJoinNotification(MISJoinNotification *msg);
    void processTerminateNotification(MISTerminateNotification *msg);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif