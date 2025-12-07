//
// Simple message definition for ring topology
//

// Basic message for ring communication
message RingMessage {
    int senderId;
    string content;
    int hopCount = 0;
}

// Message types for Fast MIS algorithm
message MISRandomValue {
    int senderId;
    double randomValue;
    int phase;
}

message MISJoinNotification {
    int senderId;
    int phase;
}

message MISTerminateNotification {
    int senderId;
    int phase;
}

// Message type for Slow MIS neighbor discovery
message MISNeighborAnnouncement {
    int senderId;
}

// Message types for Desire-Level MIS algorithm
message MISDesireLevelMessage {
    int senderId;
    double desireLevel;
    int round;
}

message MISMarkMessage {
    int senderId;
    bool isMarked;
    int round;
}
