#ifndef __DESIRELEVEL_MIS_NODE_H__
#define __DESIRELEVEL_MIS_NODE_H__

#include <omnetpp.h>
#include <set>
#include <map>
#include "message_m.h"

using namespace omnetpp;

/**
 * Implementation of the Desire-Level MIS Algorithm
 * 
 * Algorithm from "A Simple and Optimal Local Approximation Algorithm for MIS"
 * 
 * Key properties:
 * - Each node has a desire-level p_t(v) for joining MIS (initially 0.5)
 * - Effective degree d_t(v) = sum of desire-levels of neighbors
 * - Desire-level updates:
 *   - If d_t(v) >= 2: p_{t+1}(v) = p_t(v) / 2 (decrease desire)
 *   - If d_t(v) < 2: p_{t+1}(v) = min{2 * p_t(v), 0.5} (increase desire)
 * - Each round:
 *   1. Node gets marked with probability p_t(v)
 *   2. If no neighbor is marked, node joins MIS
 */
class DesireLevelMISNode : public cSimpleModule
{
private:
    // Node identification
    int nodeId;
    
    // Algorithm state
    double desireLevel;           // Current desire level p_t(v)
    double effectiveDegree;       // Sum of neighbors' desire levels
    bool isMarked;                // Whether marked in current round
    bool inMIS;                   // Whether node joined MIS
    bool terminated;              // Whether node has terminated
    int currentRound;             // Current round number
    
    // Neighbor tracking
    std::set<int> activeNeighbors;                    // Set of active neighbor IDs
    std::map<int, double> neighborDesireLevels;       // Neighbor desire levels
    std::set<int> markedNeighbors;                    // Neighbors marked in current round
    
    // Timing parameters
    double roundInterval;         // Time between rounds
    double initialStartDelay;     // Max delay for initial start
    double desireLevelSendDelay;  // Delay before sending desire level
    
    // Self-messages for scheduling
    cMessage *roundStartMsg;
    cMessage *sendDesireLevelMsg;
    cMessage *checkMarkingMsg;
    
    // Statistics tracking
    int numRoundsUntilTermination;
    
    // Helper methods
    void startNewRound();
    void sendDesireLevel();
    void performMarking();
    void checkAndJoinMIS();
    void updateDesireLevel();
    double calculateEffectiveDegree();
    void joinMIS();
    void terminate();
    void processDesireLevelMessage(MISDesireLevelMessage *msg);
    void processMarkMessage(MISMarkMessage *msg);
    void processJoinNotification(MISJoinNotification *msg);
    void processTerminateNotification(MISTerminateNotification *msg);
    void broadcastToNeighbors(cMessage *msg);
    void resetRoundData();
    
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif
