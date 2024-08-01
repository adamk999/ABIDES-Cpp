#pragma once
#include "FinancialAgent.h"
#include <vector>
#include "../util/timestamping.h"

class ExchangeAgent : public FinancialAgent {

private:
    bool reschedule;

public:
    ExchangeAgent(
        int id, 
        std::string name,
        std::string type,
        Timestamp mkt_open,
        Timestamp mkt_close,
        std::vector<std::string> symbols,
        Logger& logger,
        char book_freq = 'S',
        bool wide_book = false,
        int pipeline_delay = 40000,
        int computational_delay = 1,
        int stream_history = 0,
        bool log_orders = false,
        int random_state = -1
        ) 
        : FinancialAgent(id, name, type, random_state, logger) {
            reschedule = false;
        }
};