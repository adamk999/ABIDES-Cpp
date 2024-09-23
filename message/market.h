#pragma once
#include "message.h"
#include "../util/timestamping.h"
#include <unordered_map>


class MarketClosedMsg : public Message {
    /*
    This message is sent from an ``ExchangeAgent`` to a ``TradingAgent`` when a ``TradingAgent`` has
    made a request that cannot be completed because the market the ``ExchangeAgent`` trades
    is closed.
    */

public:
    MarketClosedMsg() : Message() {}

    std::string getName() const override {
        return "MarketClosedMsg";
    }
};

class MarketHoursRequestMsg : public Message {
    /*
    This message can be sent to an ``ExchangeAgent`` to query the opening hours of the market
    it trades. A ``MarketHoursMsg`` is sent in response.
    */
public:
    MarketHoursRequestMsg() : Message() {}

    std::string getName() const override {
        return "MarketHoursRequestMsg";
    }
};


class MarketHoursMsg : public Message {
    /*
    This message is sent by an ``ExchangeAgent`` in response to a ``MarketHoursRequestMsg``
    message sent from a ``TradingAgent``.

    Attributes:
        mkt_open: The time that the market traded by the ``ExchangeAgent`` opens.
        mkt_close: The time that the market traded by the ``ExchangeAgent`` closes.
    */
public:
    Timestamp mkt_open;
    Timestamp mkt_close;

    MarketHoursMsg(Timestamp mkt_open, Timestamp mkt_close) 
    : Message(), mkt_open(mkt_open), mkt_close(mkt_close) {}

    std::string getName() const override {
        return "MarketHoursMsg";
    }
};


class MarketClosePriceRequestMsg : public Message {
    /*      
    This message can be sent to an ``ExchangeAgent`` to request that the close price of
    the market is sent when the exchange closes. This is used to accurately calculate
    the agent's final mark-to-market value.
    */
public:
    MarketClosePriceRequestMsg() : Message() {}

    std::string getName() const override {
        return "MarketClosePriceRequestMsg";
    }
};



class MarketClosePriceMsg : public Message {
    /*      
    This message is sent by an ``ExchangeAgent`` when the exchange closes to all agents
    that habve requested this message. The value is used to accurately calculate the
    agent's final mark-to-market value.

    Attributes:
        close_prices: A mapping of symbols to closing prices.
    */

public:
    std::unordered_map<std::string, int> close_prices;

    MarketClosePriceMsg() : Message() {}
    
    std::string getName() const override {
        return "MarketClosePriceMsg";
    }

};