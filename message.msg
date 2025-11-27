//
// Simple message definition for ring topology
//

// Basic message for ring communication
message RingMessage {
    int senderId;
    string content;
    int hopCount = 0;
}
