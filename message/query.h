#pragma once
#include "Message.h"
#include <vector>
#include <unordered_map>

struct QueryMsg : public Message {
    std::string symbol;
    QueryMsg(std::string symbol) : symbol(symbol) {}

    std::string getName() const override {
        return "QueryMsg";
    }
};

struct QueryResponseMsg : public Message {
    std::string symbol;
    bool mkt_closed;
    QueryResponseMsg(std::string symbol, bool mkt_closed) : symbol(symbol), mkt_closed(mkt_closed) {}

    std::string getName() const override {
        return "QueryResponseMsg";
    }
};


struct QueryLastTradeMsg : public QueryMsg {
    // Inherited Fields:
    // symbol: str
    QueryLastTradeMsg(std::string symbol) : QueryMsg(symbol) {}

    std::string getName() const override {
        return "QueryLastTradeMsg";
    }
};


struct QueryLastTradeResponseMsg : public QueryResponseMsg {
    /* Inherited Fields:
       symbol: str
       mkt_closed: bool */    
    int last_trade;
    QueryLastTradeResponseMsg(std::string symbol, bool mkt_closed, int last_trade) : last_trade(last_trade),
    QueryResponseMsg(symbol, mkt_closed) {}

    std::string getName() const override {
        return "QueryLastTradeResponseMsg";
    }
};


struct QuerySpreadMsg : public QueryMsg {
    /* Inherited Fields:
       symbol: str */
    int depth;
    QuerySpreadMsg(std::string symbol, int depth) : QueryMsg(symbol), depth(depth) {}

    std::string getName() const override {
        return "QuerySpreadMsg";
    }
};


struct QuerySpreadResponseMsg : public QueryResponseMsg {
    /* Inherited Fields:
       symbol: str
       mkt_closed: bool */
    int depth;
    std::vector<std::tuple<int, int>> bids;
    std::vector<std::tuple<int, int>> asks;
    int last_trade;

    std::string getName() const override {
        return "QuerySpreadResponseMsg";
    }
};


struct QueryOrderStreamMsg : public QueryMsg {
    /* Inherited Fields:
       symbol: str */
    int length;
    QueryOrderStreamMsg(std::string symbol, int length) : length(length), QueryMsg(symbol) {}

    std::string getName() const override {
        return "QueryOrderStreamMsg";
    }
};


struct QueryOrderStreamResponseMsg : QueryResponseMsg {
    /* Inherited Fields:
       symbol: str
       mkt_closed: bool */
    int length;
    std::vector<std::unordered_map<std::string, int>> orders;

    std::string getName() const override {
        return "QueryOrderStreamResponseMsg";
    }
};


struct QueryTransactedVolMsg : public QueryMsg {
    // Inherited Fields:
    // symbol: str
    std::string lookback_period;
    QueryTransactedVolMsg(std::string symbol, std::string lookback_period) : 
    QueryMsg(symbol), lookback_period(lookback_period) {}

    std::string getName() const override {
        return "QueryTransactedVolMsg";
    }
};


struct QueryTransactedVolResponseMsg : QueryResponseMsg {
    /* Inherited Fields:
       symbol: str
       mkt_closed: bool */
    int bid_volume;
    int ask_volume;
    QueryTransactedVolResponseMsg(std::string symbol, bool mkt_closed, int bid_volume, int ask_volume)
    : QueryResponseMsg(symbol, mkt_closed), bid_volume(bid_volume), ask_volume(ask_volume) {}

    std::string getName() const override {
        return "QueryTransactedVolResponseMsg";
    }
};