#include "Agent.h"
#include <string>

/* The FinancialAgent class contains attributes and methods that should be available
   to all agent types (traders, exchanges, etc) in a financial market simulation.
   To be honest, it mainly exists because the base Agent class should not have any
   finance-specific aspects and it doesn't make sense for ExchangeAgent to inherit
   from TradingAgent.  Hopefully we'll find more common ground for traders and
   exchanges to make this more useful later on. */

class FinancialAgent : public Agent {

public:
    FinancialAgent(int id, std::string name, std::string type, int random_state, Logger& logger, bool log_to_file = true)
        : Agent(id, name, type, random_state, logger, log_to_file) {}

    std::string dollarise(int cents) {
        return "$" + std::to_string(cents/100) + "." + std::to_string(cents%100);
    }
};