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
  cMessage* phaseStartMsg;
  cMessage* sendRandomValueMsg;

  // Timing parameters
  double phaseInterval;
  double initialStartDelay;
  double randomValueSendDelay;

  // Statistics tracking
  int totalMessagesSent;
  int totalMessagesReceived;
  int controlMessagesReceived;
  int dataMessagesReceived;
  simtime_t algorithmStartTime;
  simtime_t algorithmEndTime;
  int initialNeighborCount;
  simsignal_t phaseSignal;
  simsignal_t clusterSizeSignal;
  simsignal_t msgOverheadSignal;
  simsignal_t convergenceTimeSignal;

 protected:
  virtual void initialize() override;
  virtual void handleMessage(cMessage* msg) override;
  virtual void finish() override;

 private:
  void startNewPhase();
  void sendRandomValue();
  void tryMakeDecision();
  void JoinMIS();
  void terminate();
  void processRandomValue(MISRandomValue* msg);
  void processJoinNotification(MISJoinNotification* msg);
  void processTerminateNotification(MISTerminateNotification* msg);
  bool shouldJoinMIS();
  void broadcastToNeighbors(cMessage* msg);
  void resetPhaseData();
  void logPhaseEnd();
};

#endif