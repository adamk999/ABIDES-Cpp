#pragma once
#include <string>

class Message {
public:
    static int uniq;
    int uniq_id;
    std::string body;
    
    enum MessageType { WAKEUP, MESSAGE } messageType;
    Timestamp requestedTime; 
    int sender;

    Message() {
    uniq_id = uniq++;
    }

    Message(std::string body) : body(body) { 
        uniq_id = uniq++;
    }

    Message(Timestamp requestedTime, int sender, MessageType type, std::string body)
        : requestedTime(requestedTime), sender(sender), messageType(type), body(body) {
        uniq_id = uniq++;   
    }

    Message(int sender, std::string body)
        : sender(sender), body(body) {
            uniq_id = uniq++;
        }

    bool operator<(const Message& other) const {
        return uniq_id < other.uniq_id;
    }
};    

class MarketDataReqMsg : public Message {
    
};
