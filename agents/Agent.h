#pragma once
#include "../util/logger.h"
#include <vector>
#include <optional>

class Kernel;
class Message;

struct LogEntry {
    Timestamp eventTime;
    std::string eventType;
    std::string event;
};


inline void writeVectorToFile(const std::vector<int>& vec, const std::string& filePath) {
    std::ofstream outFile(filePath, std::ios::trunc); // Open in truncate mode to create/overwrite the file

    if (!outFile) {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return;
    }

    for (const int& value : vec) {
        outFile << value << std::endl;
    }

    outFile.close();
}

class Agent {
private:
    int random_state;
    bool logToFile;
    std::vector<LogEntry> log;

protected:
    Kernel* kernel;
    Timestamp currentTime;
    int id;
    std::optional<std::string> name;
    std::optional<std::string> type;

public:
    Logger* logger;
    
    Agent(
        int id, 
        std::optional<std::string> name, 
        std::optional<std::string> type, 
        int random_state, 
        Logger& logger, 
        const bool& logToFile)
    : id(id), name(name), type(type), random_state(random_state), 
      logger(&logger), logToFile(logToFile) {}

    // Flow of required kernel listening methods:
    // init -> start -> (entire simulation) -> end -> terminate

    void kernelInitialising(Kernel& kernel) {
    /*  Called by kernel one time when simulation first begins.
        No other agents are guaranteed to exist at this time.

        Kernel reference must be retained, as this is the only time the
        agent can "see" it. */
        this->kernel = &kernel;
        logger->log("Agent " + std::to_string(id) + " initialising.");
    }

    void kernelStarting(Timestamp startTime) {
    /*  Called by kernel one time _after_ simulationInitializing.
        All other agents are guaranteed to exist at this time.
        startTime is the earliest time for which the agent can
        schedule a wakeup call (or could receive a message).

        Base Agent schedules a wakeup call for the first available timestamp.
        Subclass agents may override this behavior as needed. */
        setWakeup(startTime);
    }
    
    void wakeup(const Timestamp new_currentTime) {
    /*  Agents can request a wakeup call at a future simulation time using
        Agent.setWakeup().  This is the method called when the wakeup time
        arrives. */

        currentTime = new_currentTime;
        logger->log("At " + currentTime.to_string() + " agent " + std::to_string(id) +
                    name.value() + " received wakeup.");
    }

    void receiveMessage(const Timestamp new_currentTime, int senderId, const Message* message);
    /* Called each time a message destined for this agent reaches
       the front of the kernel's priority queue. currentTime is
       the simulation time at which the kernel is delivering this
       message -- the agent should treat this as "now". msg is
       an object guaranteed to inherit from the message.Message class. */


    void kernelStopping(){
    /* Called by kernel one time _before_ simulationTerminating.
        All other agents are guaranteed to exist at this time. */
    }

    void kernelTerminating() {
        std::cout << "KernelTerminating" << std::endl;
    }
    // /* Called by kernel one time when simulation terminates.
    //    No other agents are guaranteed to exist at this time. */

    // // If this agent has been maintaining a log, convert it to a Dataframe
    // // and request that the Kernel write it to disk before terminating.
    // if (log) && (log_to_file) {
    //     writeVectorToFile()
    // }


    //     dfLog = pd.DataFrame(self.log);
    //     dfLog.set_index('EventTime', inplace=True);
    //     self.writeLog(dfLog);
    // }; 

    
    void setWakeup(Timestamp requestedTime);

    int getComputationDelay();

    void setComputationDelay(const int& requestedDelay);

    void logEvent(std::string eventType, std::string event, bool appendSummaryLog = false);
    /* Adds an event to this agent's log.  The deepcopy of the Event field,
       often an object, ensures later state changes to the object will not
       retroactively update the logged event. 
       
       We can make a single copy of the object (in case it is an arbitrary
       class instance) for both potential log targets, because we don't
       alter logs once recorded. */

    void sendMessage(int recipientID, Message msg, int delay = 0);
};
