#include "../util/timestamping.h"
#include "../Kernel.h"

void Agent::setWakeup(Timestamp requestedTime) {
    kernel->setWakeup(id, requestedTime);
}

int Agent::getComputationDelay() {
    return kernel->getAgentComputeDelay(id);
}

void Agent::setComputationDelay(const int& requestedDelay) {
    kernel->setAgentComputeDelay(id, requestedDelay);
}

void Agent::sendMessage(int recipientID, Message msg, int delay) {
    kernel->sendMessage(id, recipientID, msg, delay = delay);
}

void Agent::logEvent(std::string eventType, std::string event, bool appendSummaryLog) {
    LogEntry e;
    e.event = event;
    e.eventTime = currentTime;
    e.eventType = eventType;
    log.push_back(e);

    if (appendSummaryLog) { kernel->appendSummaryLog(id, eventType, e); }
}

void Agent::receiveMessage(const Timestamp new_currentTime, int senderId, const Message* message) {
    currentTime = new_currentTime;
    logger->log("At " + new_currentTime.to_string() + ", agent " + std::to_string(id) + name.value() + " received: " + message->getName());
    }