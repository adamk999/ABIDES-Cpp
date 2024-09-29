#pragma once
#include <string>
#include "TradingAgent.h"
#include "../util/logger.h"
#include "../util/oracles/Oracle.h"

class Message;
class Timestamp;

class NoiseAgent : public TradingAgent {
/*
    Noise agent implements a simple strategy: agent wakes up once and places 1 order.
*/

private:
    std::string symbol;
    bool order_size_model;   // Not implemented yet.
    Timestamp wakeup_time;

    bool trading;
    std::string state;
    Timestamp prev_wake_time;
    int size;
    Oracle oracle;

public:
    NoiseAgent(
        int id,
        std::string symbol,
        Logger& logger,
        bool order_size_model = false,   // Not implemented yet.
        Timestamp wakeup_time = Timestamp(),
        bool log_orders = false,
        int starting_cash = 100000,
        std::string name = "",
        std::string type = "",
        int random_state = 1
    );

    void kernelStarting(Timestamp startTime);
    
    void kernelStopping();
        
    void wakeup(Timestamp currentTime);
    /*
        Arguments:
            current_time: The time that this agent was woken up by the kernel.

        Returns:
            For the sake of subclasses, TradingAgent now returns a boolean
            indicating whether the agent is "ready to trade" -- has it received
            the market open and closed times, and is the market not already closed.
    */
        
    void placeOrder();
     
    void receiveMessage(Timestamp current_time, int sender_id, Message& message);
      
    Timestamp getWakeFrequency();

      
};