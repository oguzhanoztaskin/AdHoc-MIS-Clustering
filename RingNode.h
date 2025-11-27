#ifndef __RINGNODE_H
#define __RINGNODE_H

#include <omnetpp.h>
#include "message_m.h"

using namespace omnetpp;

class RingNode : public cSimpleModule {
private:
    int nodeId;
    cMessage *selfMsg;
    int messageCount;
    
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif