#ifndef __FASTMISNODE_H
#define __FASTMISNODE_H

#include <omnetpp.h>
#include <map>
#include <set>
#include "message_m.h"

using namespace omnetpp;

class FastMISNode : public cSimpleModule {
private:
    int nodeId;
    int currentPhase;
    bool inMIS;
    bool terminated;
    double myRandomValue;
    
    // Tracking neighbors and their random values
    std::set<int> activeNeighbors;
    std::map<int, double> neighborRandomValues;
    std::set<int> neighborsInMIS;
    
    // Self-scheduling messages
    cMessage *phaseStartMsg;
    cMessage *randomValueTimeoutMsg;
    cMessage *decisionTimeoutMsg;
    
    // Timing parameters
    double phaseInterval;
    double randomValueTimeout;
    double decisionTimeout;
    
    // Statistics
    int totalPhases;
    
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    
private:
    void startNewPhase();
    void sendRandomValue();
    void makeDecision();
    void joinMIS();
    void terminate();
    void processRandomValue(MISRandomValue *msg);
    void processJoinNotification(MISJoinNotification *msg);
    void processTerminateNotification(MISTerminateNotification *msg);
    bool shouldJoinMIS();
    void broadcastToNeighbors(cMessage *msg);
    void resetPhaseData();
};

#endif