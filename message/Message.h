#pragma once
#include <string>  

class Message {
    /*
    The base Message class no longer holds envelope/header information, however any
    desired information can be placed in the arbitrary body.

    Delivery metadata is now handled outside the message itself.

    The body may be overridden by specific message type subclasses.
    */

    /* The autoincrementing variable here will ensure that, when Messages are due for
       delivery at the same time step, the Message that was created first is delivered
       first. (Which is not important, but Python 3 requires a fully resolved chain of
       priority in all cases, so we need something consistent) We might want to generate
       these with stochasticity, but guarantee uniqueness somehow, to make delivery of
       orders at the same exact timestamp "random" instead of "arbitrary" (FIFO among
       tied times) as it currently is. */

public:
    static int uniq;
    int uniq_id;

    Message() {
        uniq_id = uniq++;
    }
    
    virtual std::string getName() const {
        return "Message";
    }

    virtual ~Message() = default;
    
};

class WakeupMsg : public Message {
    /*
    Empty message send to agents to wake them up.
    */

    std::string getName() const override {
        return "WakeupMsg";
    }
};