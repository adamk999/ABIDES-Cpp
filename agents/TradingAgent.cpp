#pragma once
#include <tuple>
#include "TradingAgent.h"
#include "../kernel.h"
#include "ExchangeAgent.h"


TradingAgent::TradingAgent(
    int id, 
    std::string name, 
    std::string type, 
    int random_state, 
    const int starting_cash = 100000, 
    Logger& logger,
    bool log_orders = false,
    bool log_to_file = true
    )
    : FinancialAgent(id, name, type, random_state, logger, log_to_file), 
        log_orders(log_orders), starting_cash(starting_cash) {
        mkt_open = Timestamp();
        mkt_close = Timestamp();

        // TradingAgent has constants to support simulated market orders.
        MKT_BUY = pow(10, 10);
        MKT_SELL = 0;

        /* The base TradingAgent will track its holdings and outstanding orders.
            Holdings is a dictionary of symbol -> shares.  CASH is a special symbol
            worth one cent per share.  Orders is a dictionary of active, open orders
            (not cancelled, not fully executed) keyed by order_id. */
        holdings["CASH"] = starting_cash;
        
        nav_diff = 0;
        basket_size = 0;

        /* For special logging at the first moment the simulator kernel begins
            running (which is well after agent init), it is useful to keep a simple
            boolean flag. */
        first_wake = true;

        /* Remember whether we have already passed the exchange close time, as far
            as we know. */
        mkt_closed = false;
}

void TradingAgent::kernelStarting(const int& startTime) {
    // kernel is set in Agent.kernelInitializing().
    logEvent("STARTING_CASH", std::to_string(starting_cash), true);

    /* Find an exchange with which we can place orders.  It is guaranteed
       to exist by now (if there is one). */
    exchangeID = kernel->findAgentByType("ExchangeAgent");

    logger->log("Agent {} requested agent of type Agent.ExchangeAgent.  Given Agent ID: {}" + 
               std::to_string(id) + std::to_string(exchangeID));

    // Request a wake-up call as in the base Agent.
    FinancialAgent::kernelStarting(startTime);
}

void TradingAgent::kernelStopping() {
    // Always call parent method to be safe.
    Agent::kernelStopping();

    // Print end of day holdings.
    logEvent("FINAL_HOLDINGS", fmtHoldings(holdings));
    logEvent("FINAL_CASH_POSITION", std::to_string(holdings["CASH"]), true);

    // Mark to market.
    cash = markToMarket(holdings);

    logEvent("ENDING_CASH", std::to_string(cash), true);
    std::cout << "Final holdings for" << name << ": " << fmtHoldings(holdings) <<  " Marked to market: " << cash << std::endl;
    
    // Record final results for presentation/debugging.
    std::string mytype = type;
    int gain = cash - starting_cash;
    
    // Check mytype exists in meanResultByAgentType.
    auto it = kernel->meanResultByAgentType.find(mytype);
    if (it != kernel->meanResultByAgentType.end()) {
        // Key exists, update value.
        kernel->meanResultByAgentType[mytype] += gain;
        kernel->agentCountByType[mytype] += 1; 
    }
    else {
        // Key does not exist, create new entry.
        kernel->meanResultByAgentType[mytype] = gain;
        kernel->agentCountByType[mytype] = 1;
    }
}

bool TradingAgent::wakeup(const Timestamp currentTime) {
    Agent::wakeup(currentTime);

    if (first_wake) {
      // Log initial holdings.
      logEvent("HOLDINGS_UPDATED", fmtHoldings(holdings));
      first_wake = false;
    }

    if (not mkt_open.isValid()) {
      // Ask our exchange when it opens and closes.
      sendMessage(exchangeID, Message(id, "WHEN_MKT_OPEN"));
      sendMessage(exchangeID, Message(id, "WHEN_MKT_CLOSE"));
    }

    /* For the sake of subclasses, TradingAgent now returns a boolean
       indicating whether the agent is "ready to trade" -- has it received
       the market open and closed times, and is the market not already closed. */
    return (mkt_open.isValid() and mkt_close.isValid()) and not mkt_closed;
}

void TradingAgent::requestDataSubscription(std::string symbol, const int levels, const int freq) {
      sendMessage(recipientID = exchangeID,
                  msg = Message({"msg": "MARKET_DATA_SUBSCRIPTION_REQUEST",
                                 "sender": self.id, "symbol": symbol, "levels": levels, "freq": freq}));
}

void TradingAgent::cancelDataSubscription(std::string symbol) {
    sendMessage(recipientID=exchangeID,
                     msg=Message({"msg": "MARKET_DATA_SUBSCRIPTION_CANCELLATION",
                                  "sender": self.id, "symbol": symbol}));
}

void TradingAgent::receiveMessage(Timestamp currentTime, std::string msg) { }
    // super().receiveMessage(currentTime, msg) // need to check that this actually just calls the function..

    // // Do we know the market hours?
    // bool had_mkt_hours = mkt_open is not none and mkt_close is not none;

    // // Record market open or close times.
    // if (msg.body["msg"] == "WHEN_MKT_OPEN")
    //     mkt_open = msg.body["data"]
    //     logger.log("Recorded market open: " + kernel->fmtTime(mkt_open));

    // elif msg.body["msg"] == "WHEN_MKT_CLOSE":
    //     mkt_close = msg.body["data"]
    //     logger.log("Recorded market close: {}", self.kernel.fmtTime(self.mkt_close))

    // elif msg.body['msg'] == "ORDER_EXECUTED":
    //   # Call the orderExecuted method, which subclasses should extend.  This parent
    //   # class could implement default "portfolio tracking" or "returns tracking"
    //   # behavior.
    //   order = msg.body['order']

    //   self.orderExecuted(order)

    // elif msg.body['msg'] == "ORDER_ACCEPTED":
    //   # Call the orderAccepted method, which subclasses should extend.
    //   order = msg.body['order']

    //   self.orderAccepted(order)

    // elif msg.body['msg'] == "ORDER_CANCELLED":
    //   # Call the orderCancelled method, which subclasses should extend.
    //   order = msg.body['order']

    //   self.orderCancelled(order)

    // elif msg.body['msg'] == "MKT_CLOSED":
    //   # We've tried to ask the exchange for something after it closed.  Remember this
    //   # so we stop asking for things that can't happen.

    //   self.marketClosed()

    // elif msg.body['msg'] == 'QUERY_LAST_TRADE':
    //   # Call the queryLastTrade method, which subclasses may extend.
    //   # Also note if the market is closed.
    //   if msg.body['mkt_closed']: self.mkt_closed = True

    //   self.queryLastTrade(msg.body['symbol'], msg.body['data'])

    // elif msg.body['msg'] == 'QUERY_SPREAD':
    //   # Call the querySpread method, which subclasses may extend.
    //   # Also note if the market is closed.
    //   if msg.body['mkt_closed']: self.mkt_closed = True

    //   self.querySpread(msg.body['symbol'], msg.body['data'], msg.body['bids'], msg.body['asks'], msg.body['book'])

    // elif msg.body['msg'] == 'QUERY_ORDER_STREAM':
    //   # Call the queryOrderStream method, which subclasses may extend.
    //   # Also note if the market is closed.
    //   if msg.body['mkt_closed']: self.mkt_closed = True

    //   self.queryOrderStream(msg.body['symbol'], msg.body['orders'])

    // elif msg.body['msg'] == 'QUERY_TRANSACTED_VOLUME':
    //   if msg.body['mkt_closed']: self.mkt_closed = True
    //   self.query_transacted_volume(msg.body['symbol'], msg.body['transacted_volume'])

    // elif msg.body['msg'] == 'MARKET_DATA':
    //   self.handleMarketData(msg)

    // # Now do we know the market hours?
    // have_mkt_hours = self.mkt_open is not None and self.mkt_close is not None

    // # Once we know the market open and close times, schedule a wakeup call for market open.
    // # Only do this once, when we first have both items.
    // if have_mkt_hours and not had_mkt_hours:
    //   # Agents are asked to generate a wake offset from the market open time.  We structure
    //   # this as a subclass request so each agent can supply an appropriate offset relative
    //   # to its trading frequency.
    //   ns_offset = self.getWakeFrequency()

    //   self.setWakeup(self.mkt_open + ns_offset)


std::string TradingAgent::fmtHoldings(const std::unordered_map<std::string, int>& holdings) {
    std::ostringstream oss;
    oss << "{ ";
    
    // Ensure there's always a CASH entry.
    auto cashIt = holdings.find("CASH");
    int cashValue = 0;
    if (cashIt != holdings.end()) {
        cashValue = cashIt->second;
    }

    // Iterate over the holdings and format the string.
    for (const auto& [key, value] : holdings) {
        if (key == "CASH") continue;  // Skip the CASH entry for now.
        oss << key << ": " << value << ", ";
    }
    
    oss << "CASH: " << cashValue;
    oss << " }";
    return oss.str();
}

int TradingAgent::markToMarket(std::unordered_map<std::string, int>& holdings, bool use_midpoint = false) {
    auto cashIt = holdings.find("CASH");
    int cashValue = 0;
    if (cashIt != holdings.end()) {
        cashValue = cashIt->second;
    }

    cash += basket_size*nav_diff;

    int value = 0;
    for (const auto& [symbol, shares] : holdings) {
        if (symbol == "CASH") { continue; }

        if (use_midpoint) {
            auto [bid, ask, midpoint] = getKnownBidAskMidpoint(symbol); // return std::make_tuple(bid, ask, midpoint);
            if (bid == -1 || ask == -1 || midpoint == -1) {
                value = last_trade[symbol] * shares;
            }
            else {
                value = midpoint * shares;
            }
        }
        else {
            int value = last_trade[symbol] * shares;
        }
        cash += value;
        logger->log("MARK_TO_MARKET" + std::to_string(shares) + symbol + " @ "  + last_trade[symbol], + std::to_string(value));
    }
    return cash;
}

std::tuple<int, int, int> TradingAgent::getKnownBidAskMidpoint(std::string symbol) {
    int bid = -1;
    int ask = -1;
    int mid = -1;
    return std::make_tuple(bid, ask, mid);
}