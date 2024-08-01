#include "../util/timestamping.h"
#include "Agent.h"
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