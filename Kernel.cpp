#include "util/timestamping.h"
#include "Kernel.h"
#include "util/oracles/ExternalFileOracle.h"
#include "util/util.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <ctime>

// Initialise message static id var to 0.
int Message::uniq = 0;

Kernel::Kernel(const std::string& kernel_name, const int& random_state, Logger& logger) :
    kernel_name(kernel_name), random_state(random_state), logger(logger)
{
    kernelWallClockStart = time(0);

    logger.log("Kernel initialised.");
}

std::unordered_map<std::string, std::string> Kernel::runner(
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
        )
{
    // Agents must be a list of agents for the simulation.
    this->agents = agents;

    /* The kernel start and stop time (first and last timestamp in
        the simulation, separate from anything like exchange open/close). */
    this->startTime = startTime;
    this->stopTime = stopTime;

    // Global seed, NOT used for anything agent-related.
    this->seed = seed;

    this->skip_log = skip_log;
    this->oracle = oracle;
    
    /* The kernel maintains a current time for each agent to allow
        simulation of per-agent computation delays.  The agent's time
        is pushed forward (see below) each time it awakens, and it
        cannot receive new messages/wakeups until the global time
        reaches the agent's time.

        This also nicely enforces agents being unable to act before
        the simulation startTime. */
    int n_agents = agents.size();
    agentCurrentTimes.resize(n_agents, currentTime);

    /* agentComputationDelays is in nanoseconds, starts with a default
        value from config, and can be changed by any agent at any time
        (for itself only).  It represents the time penalty applied to
        agent each time it is awakened (wakeup or recvMsg). The
        penalty applies _after_ the agent acts, before it may act again. */
    agentComputationDelays.resize(n_agents, defaultComputationalDelay);

    agentLatency.resize(n_agents, std::vector<int>(n_agents, defaultLatency));

    currentAgentAdditionalDelay = 0;

    logger.log("Kernel started.");
    logger.log("Simulation started.");
    
    /*  Note that num_simulations has not yet been really used or tested
        for anything.  Instead we have been running multiple simulations
        with coarse parallelization from a shell script.*/
    for (int sim=0; sim<num_simulations; sim++)
    {
        logger.log("Starting sim " + std::to_string(sim));
        /* Event notification for kernel init (agents should not try to
            communicate with other agents, as order is unknown). Agents
            should initialize any internal resources that may be needed
            to communicate with other agents during agent.kernelStarting().
            Kernel passes self-reference for agents to retain, so they can
            communicate with the kernel in the future (as it does not have
            an agentID). */
            
        logger.log("––– Agent.kernelInitialising() ---");
        for (int i=0; i<n_agents; i++)
        {
            this->agents[i].kernelInitialising(*this);
        }

        /* Event notification for kernel start (agents may set up
            communications or references to other agents, as all agents
            are guaranteed to exist now).  Agents should obtain references
            to other agents they require for proper operation (exchanges,
            brokers, subscription services...).  Note that we generally
            don't (and shouldn't) permit agents to get direct references
            to other agents (like the exchange) as they could then bypass
            the Kernel, and therefore simulation "physics" to send messages
            directly and instantly or to perform disallowed direct inspection
            of the other agent's state.  Agents should instead obtain the
            agent ID of other agents, and communicate with them only via
            the Kernel.  Direct references to utility objects that are not
            agents are acceptable (e.g. oracles). */

        logger.log("––– Agent.kernelStarting() ---");
        for (int i=0; i<n_agents; i++) {
            this->agents[i].kernelStarting(startTime);
        }

        // Set the kernel to its startTime.
        currentTime = startTime;
        logger.log("––– Kernel Clock started ---");
        logger.log("Kernel.currentTime is now" + currentTime.to_string());

        // Start processing the Event Queue.
        logger.log("––– Kernel Event Queue begins ---");
        logger.log("Kernel will start processing messages.  Queue length: " +  std::to_string(messages.size()));

        // Track starting wall clock time and total message count.
        eventQueueWallClockStart = time(0);
        ttl_messages = 0;
        
        /* Process messages until there aren't any (at which point there never can
            be again, because agents only "wake" in response to messages), or until
            the kernel stop time is reached. */

        while (not messages.empty() and currentTime.isValid() and (currentTime <= stopTime)) {        
            // Get the next message in timestamp order (delivery time) and extract it.
            Message msg = messages.top();
            messages.pop();

            // Periodically print the simulation time and total messages, even if muted.
            if (ttl_messages % 100000 == 0)
            {
                std::ostringstream oss;
                oss << "\n--- Simulation time: " << currentTime.to_string() << ", messages processed: " 
                << ttl_messages << ", wallclock elapsed: " << time(0) - eventQueueWallClockStart;
                logger.log(oss.str()); // Log the message using Logger
            }
            
            logger.log("\n--- Kernel Event Queue pop ---");
            logger.log("Kernel handling " + std::to_string(msg.messageType) + " message for agent " 
            + std::to_string(msg.sender) + " at time " + currentTime.to_string());

            ttl_messages ++;
            
            // In between messages, always reset the currentAgentAdditionalDelay.
            currentAgentAdditionalDelay = 0;


            // Dispatch message to agent.
            if (msg.messageType == Message::MessageType::WAKEUP) {
                // Who requested this wakeup call?
                int agent = msg.sender;
                
                /* Test to see if the agent is already in the future.  If so,
                    delay the wakeup until the agent can act again. */
                if (agentCurrentTimes[agent] > currentTime)
                {
                    // Push the wakeup call back into th PQ with a new time.
                    messages.push(Message(agentCurrentTimes[agent], agent, msg.messageType, msg.body));
                    logger.log("Agent in future: wakeup requested for " + agentCurrentTimes[agent].to_string());
                    continue;
                }
                // Set agent's current time to global current time for start of processing.
                agentCurrentTimes[agent] = currentTime;

                // Wake the agent.
                agents[agent].wakeup(currentTime);

                // Delay the agent by its computation delay plus any transient additional delay requested.
                agentCurrentTimes[agent] += agentComputationDelays[agent] + currentAgentAdditionalDelay;
            
                logger.log("After wakeup return, agent " + std::to_string(agent) + " delayed from " 
                            + " to" + agentCurrentTimes[agent].to_string());
            }

            else if (msg.messageType == Message::MessageType::MESSAGE) {
                // Who is receiving this message?
                int agent = msg.sender;

                // Test to see if the agent is already in the future. If so
                // delay the message until the agent can act again.
                if (agentCurrentTimes[agent] > currentTime)
                {
                    // Push the message back into the PQ with a new time.
                    messages.push(Message(agentCurrentTimes[agent], agent, msg.messageType, msg.body));
                    logger.log("Agent in future: message requed for " + agentCurrentTimes[agent].to_string());
                    continue;
                }

                // Set agent's current time to global current time for start of processsing.
                agentCurrentTimes[agent] = currentTime;

                // Deliver the message.
                agents[agent].receiveMessage(currentTime, msg.body);

                // Delay the agent by its computation plus any transient additoinal delay requested.
                agentCurrentTimes[agent] += agentComputationDelays[agent] + currentAgentAdditionalDelay;

                logger.log("After receiveMessage return, agent " + std::to_string(agent) + " delayed from "
                            + currentTime.to_string() + " to " + agentCurrentTimes[agent].to_string());
            }

            else {
                throw std::runtime_error("Unknown message type found in queue: \n currentTime: " + currentTime.to_string() +
                                    ", messageType: " + std::to_string(msg.messageType));
            }
        }
        if (messages.empty()) { logger.log("\n--- Kernel Event Queue empty ---"); }

        if (currentTime.isValid() && (currentTime > stopTime)) { logger.log("\n--- Kernel Stop Time surpassed ---"); }

        // Record wall clock stop time and elapsed time for stats at the end.
        int eventQueueWallClockStop = time(0);

        eventQueueWallClockElapsed = eventQueueWallClockStop - eventQueueWallClockStart;

        /* Event notification for kernel end (agents may communicate with
            other agents, as all agents are still guaranteed to exist).
            Agents should not destroy resources they may need to respond
            to final communications from other agents. */
        logger.log("\n--- Agent.kernelStopping() ---");
        
        for (int id = 0; id < agents.size(); id++) {
            agents[id].kernelStopping();
        }

        /* Event notification for kernel termination (agents should not
            attempt communication with other agents, as order of termination
            is unknown). Agents should clean up all used resources as the
            simulation program may not actually terminate if num_simulations > 1. */
        logger.log("\n--- Agent.kernelTerminating() ---");

        for (int id = 0; id < agents.size(); id++) {
            agents[id].kernelTerminating();
        }
        
        std::cout << "Event Queue elapsed: " << eventQueueWallClockElapsed << ", messages: " << ttl_messages 
                    << ", messages per second: IMPLEMENT" << std::endl;

        logger.log("Ending sim " + std::to_string(sim));
    }
    // The Kernel adds a handful of custom state results for all simulations,
    // which configurations may use, print, log, or discard.
    custom_state["kernel_event_queue_elapsed_wallclock"] = std::to_string(eventQueueWallClockElapsed);
    
    auto maxElementIt = std::max_element(agentCurrentTimes.begin(), agentCurrentTimes.end());
    if (maxElementIt != agentCurrentTimes.end()) {
        custom_state["kernel_slowest_agent_finish_time"] = maxElementIt->to_string();
    }

    /* Agents will request the Kernel to serialize their agent logs, usually
        during kernelTerminating, but the Kernel must write out the summary log itself. */
    writeSummaryLog();

    /* This should perhaps be elsewhere, as it is explicitly financial, but it
        is convenient to have a quick summary of the results for now. */
    std::cout << "Mean ending value by agent type:" << std::endl;
    for (const auto& entry : meanResultByAgentType) {
        // IMPLEMENT.   
    }

    std::cout << "Simulation ending!" << std::endl;

    return custom_state;
}
void Kernel::sendMessage(
    const int& sender, 
    const int& recipient, 
    const Message msg,
    int delay
    ) {
    /* Apply the agent's current computation delay to effectively "send" the message
       at the END of the agent's current computation period when it is done "thinking".
       NOTE: sending multiple messages on a single wake will transmit all at the same
       time, at the end of computation.  To avoid this, use Agent.delay() to accumulate
       a temporary delay (current cycle only) that will also stagger messages.

       The optional pipeline delay parameter DOES push the send time forward, since it
       represents "thinking" time before the message would be sent.  We don't use this
       for much yet, but it could be important later.

       This means message delay (before latency) is the agent's standard computation delay
       PLUS any accumulated delay for this wake cycle PLUS any one-time requested delay
       for this specific message only. */
    Timestamp sentTime(currentTime + agentComputationDelays[sender] + currentAgentAdditionalDelay + delay);
    
    /* Apply communication delay per the agentLatencyModel, if defined, or the agentLatency
       matrix [sender][recipient] otherwise. */
    // TODO: Implement agency latency model
    int latency = agentLatency[sender][recipient];
    double noise = genRandInt(0,3);
    Timestamp deliverAt(sentTime + latency + noise);

}


void Kernel::setWakeup(const int& sender, Timestamp requestedTime) {
    if (requestedTime == 0) { requestedTime = currentTime + 1000; }

    if (currentTime.isValid() && (requestedTime < currentTime)) {
        std::ostringstream errorMessage;
        errorMessage << "setWakeup() called with requested time not in future\n"
                     << "currentTime: " << currentTime.to_string()
                     << "requestedTime: " << requestedTime.to_string();
        throw std::runtime_error(errorMessage.str());
    }

    logger.log("Kernel adding wakeup for agent " + std::to_string(sender) + " at time " 
    + requestedTime.to_string());

    messages.push(Message(requestedTime, sender, Message::WAKEUP, "NA"));
}

int Kernel::getAgentComputeDelay(const int& sender) {
    return agentComputationDelays[sender];
}

void Kernel::setAgentComputeDelay(const int& sender, const int requestedDelay) {
    if (requestedDelay < 0)
    {
        std::ostringstream errorMessage;
        errorMessage << "Requested computation delay must be non-negative nanoseconds.\n"
                     << "requestedDelay:" << requestedDelay;
        throw std::runtime_error(errorMessage.str());
    }
    agentComputationDelays[sender] = requestedDelay;
}

void Kernel::writeSummaryLog() {
    std::cout << "Writing Summary Log" << std::endl;
}

int main()
{

    //Kernel kernel("first_kernel", 1);
    //std::vector<Agent> agents(1);
    //gents[0] = Agent(10, "first_agent", "na", 30);
    ExternalFileOracle oracle("AAPL", "data/AAPL.csv");

    //kernel.runner(agents, 10, 11, 1, 1, 50, false, oracle, "/directory");

    // Instantiate the current timestamp
    Timestamp now = Timestamp::now();

    // Print the timestamp in a readable format
    std::cout << "Current timestamp (string): " << now.to_string() << std::endl;

    // Print the timestamp in nanoseconds since epoch
    std::cout << "Current timestamp (nanoseconds since epoch): " << now.to_nanoseconds() << std::endl;

    
    return 0;
}