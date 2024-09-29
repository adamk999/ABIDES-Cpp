#include "NoiseAgent.h"
#include "../util/util.h"
#include "../Kernel.h"
#include "../message/orders.h"

NoiseAgent::NoiseAgent(
        int id,
        std::string symbol,
        Logger& logger,
        bool order_size_model,   // Not implemented yet.
        Timestamp wakeup_time,
        bool log_orders,
        int starting_cash,
        std::string name,
        std::string type,
        int random_state
    ) : TradingAgent(id, name, type, random_state, logger, starting_cash, log_orders),
        wakeup_time(wakeup_time), symbol(symbol), order_size_model(order_size_model) {

        // The agent uses this to track whether it has begun its strategy or is still
        // handling pre-market tasks.
        trading = false;

        // The agent begins in its "complete" state, not waiting for
        // any special event or condition.
        state = "AWAITING_WAKEUP";

        // The agent must track its previous wake time, so it knows how many time
        // units have passed.
        prev_wake_time = Timestamp();

        size = genRandInt(20, 50);
}

void NoiseAgent::kernelStarting(Timestamp startTime) {
    // kernel is set in Agent.kernel_initializing()
    // exchange_id is set in TradingAgent.kernel_starting()
    TradingAgent::kernelStarting(startTime);

    oracle = kernel->oracle;
}
    
void NoiseAgent::kernelStopping() {
    // Always call parent method to be safe.
    TradingAgent::kernelStopping();

    // Fix the problem of logging an agent that has not waken up.
    try {
        // Attempt to get bid/ask values
        auto [bid, bid_vol, ask, ask_vol] = getKnownBidAsk(symbol);
 
        // Print end of day valuation.
        int H = int(getHoldings(symbol)/100);

        int rT;

        if (bid != -1 and ask != -1) {
            rT = int(bid + ask) / 2;
        }
        else {
            auto it = last_trade.find(symbol);
            rT = it->second;
        }

        // Final (real) fundamental value times shares held.
        float surplus = rT * H;

        logger->log("Surplus after holdings: " + std::to_string(surplus));

        // Add ending cash value and subtract starting cash value.
        surplus += holdings["CASH"] - starting_cash;
        surplus = surplus / starting_cash;

        logEvent("FINAL_VALUATION", std::to_string(surplus), true);

        logger->log(
            name + "final report. Holdings: " + std::to_string(H) + ", end cash: " + std::to_string(holdings["CASH"])
            + ", start cash: " + std::to_string(starting_cash) + ", final fundamental: " 
            + std::to_string(rT) + ", surplus: " + std::to_string(surplus)
        );
    }
    catch (const std::out_of_range& e) {
        logEvent("FINAL_VALUATION", std::to_string(starting_cash), true);
    }
}
    
void NoiseAgent::wakeup(Timestamp currentTime) {
    TradingAgent::wakeup(currentTime);
        state = "INACTIVE";

        if (not mkt_open.isValid() or not mkt_close.isValid()) {
            // TradingAgent handles discovery of exchange times.
            return;
        }
        else {
            if (not trading) {
                trading = true;

                // Time to start trading!
                logger->log(name + " is ready to start trading now.");
            }
        }

        // Steady state wakeup behavior starts here.

        // If we've been told the market has closed for the day, we will only request
        // final price information, then stop.
        if (mkt_closed && daily_close_price.find(symbol) != daily_close_price.end()) {
            // Market is closed and we already have the daily close price.
            return;
        }

        if (wakeup_time > currentTime) {
            setWakeup(wakeup_time);
            return;
        }

        if (mkt_closed && daily_close_price.find(symbol) == daily_close_price.end()) {
            getCurrentSpread(symbol);
            state = "AWAITING_SPREAD";
            return;
        }
        // if (true) {
        //     getCurrentSpread(symbol);
        //     state = "AWAITING_SPREAD";
        // }
        
        else {
            state = "ACTIVE";
        }
}
    
void NoiseAgent::placeOrder() {
    // Place order in random direction at mid.
    int buy_indicator = genRandInt(0,2);

    auto [bid, bid_vol, ask, ask_vol] = getKnownBidAsk(symbol);

    if (order_size_model != false) {
        // self.size = self.order_size_model.sample(random_state=self.random_state) 
    }
            
    if (size > 0) {
        if (buy_indicator == 1 && ask != -1) {
            placeLimitOrder(symbol, size, Side(Side::Type::BID), ask);
        }
        else if (buy_indicator == 0 && bid != -1) {
            placeLimitOrder(symbol, size, Side(Side::Type::ASK), bid);
        }
    }
}
    
void NoiseAgent::receiveMessage(Timestamp currentTime, int sender_id, Message& message) {
        // Parent class schedules market open wakeup call once market open/close times are known.
        TradingAgent::receiveMessage(currentTime, sender_id, message);

        // We have been awakened by something other than our scheduled wakeup.
        // If our internal state indicates we were waiting for a particular event,
        // check if we can transition to a new state.

        if (state == "AWAITING_SPREAD") {
            // We were waiting to receive the current spread/book.  Since we don't currently
            // track timestamps on retained information, we rely on actually seeing a
            // QUERY_SPREAD response message.

            if (message.getName() == "QuerySpreadResponseMsg") {
                // This is what we were waiting for.

                // But if the market is now closed, don't advance to placing orders.
                if (mkt_closed) {
                    return;
                }
                // We now have the information needed to place a limit order with the eta
                // strategic threshold parameter.
                placeOrder();
                state = "AWAITING_WAKEUP";
            }
        }
}
// Internal state and logic specific to this agent subclass.
Timestamp NoiseAgent::getWakeFrequency() {
        return Timestamp(genRandInt(0, 100));
}