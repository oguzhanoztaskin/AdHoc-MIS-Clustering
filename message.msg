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
