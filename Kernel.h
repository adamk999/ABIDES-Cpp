#pragma once
#include "message/Message.h"
#include "agents/Agent.h"
#include <string>
#include <queue>
#include <iostream>
#include <unordered_map>
#include "util/oracles/Oracle.h"

class Agent;
struct LogEntry;

class Kernel
{
private:
    std::vector<Agent> agents;
    // Member variable to store key-value pairs
    std::unordered_map<std::string, std::string> custom_state;
    bool skip_log;

    std::vector<int> agentComputationDelays;
    std::vector<Timestamp> agentCurrentTimes;

    int eventQueueWallClockStart;
    int eventQueueWallClockElapsed;
    int ttl_messages;
    int currentAgentAdditionalDelay;
    std::vector<std::vector<int> > agentLatency;

    Logger& logger;

    void writeSummaryLog();

public:
    std::priority_queue<Message> messages;
    std::string kernel_name;
    std::string summaryLog[1000];
    std::unordered_map<std::string, int> meanResultByAgentType;
    std::unordered_map<std::string, int> agentCountByType;

    int random_state;
    int kernelWallClockStart;
    Timestamp currentTime;

    Timestamp startTime; 
    Timestamp stopTime;
    int seed;
    int num_stimulations;
    int defaultComputationalDelay;

    Oracle oracle;

    Kernel(
        const std::string& kernel_name, 
        const int& random_state, 
        Logger& logger
        );

    std::unordered_map<std::string, std::string> runner(
        std::vector<Agent>& agents, 
        int startTime, 
        int stopTime,
        int seed,
        int num_simulations,
        int defaultComputationalDelay,
        int defaultLatency,
        bool skip_log,
        Oracle& oracle,
        std::string log_dir
        );
    
    void sendMessage(
        const int& sender, 
        const int& recipient, 
        const Message msg,
        int delay = 0
        );
    /* Called by an agent to send a message to another agent.  The kernel
       supplies its own currentTime (i.e. "now") to prevent possible
       abuse by agents. The kernel will handle computational delay penalties
       and/or network latency. The message must derive from the message.Message class.
       The optional delay parameter represents an agent's request for ADDITIONAL
       delay (beyond the Kernel's mandatory computation + latency delays) to represent
       parallel pipeline processing delays (that should delay the transmission of messages
       but do not make the agent "busy" and unable to respond to new messages). */

    void setWakeup(const int& sender, Timestamp requestedTime);
    /* Called by an agent to receive a "wakeup call" from the kernel
       at some requested future time.  Defaults to the next possible
       timestamp.  Wakeup time cannot be the current time or a past time.
       Sender is required and should be the ID of the agent making the call.
       The agent is responsible for maintaining any required state; the
       kernel will not supply any parameters to the wakeup() call. */

    int getAgentComputeDelay(const int& sender);

    void setAgentComputeDelay(const int& sender, const int requestedDelay);

    void appendSummaryLog(int id, std::string eventType, LogEntry e);

    int findAgentByType(std::string type);
    /* Called to request an arbitrary agent ID that matches the class or base class
       passed as "type".  For example, any ExchangeAgent, or any NasdaqExchangeAgent.
       This method is rather expensive, so the results should be cached by the caller! */

};