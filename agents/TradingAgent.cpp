#include "TradingAgent.h"
#include <limits>
#include "../kernel.h"
#include "ExchangeAgent.h"
#include "../message/market.h"
#include "../message/query.h"
#include "../message/orders.h"

TradingAgent::TradingAgent(
    int id, 
    std::string name, 
    std::string type, 
    int random_state, 
    Logger& logger,
    const int starting_cash, 
    bool log_orders,
    bool log_to_file
    )
    : FinancialAgent(id, name, type, random_state, logger, log_to_file), 
        log_orders(log_orders), starting_cash(starting_cash) {
        mkt_open = Timestamp();
        mkt_close = Timestamp();

        // TradingAgent has constants to support simulated market orders.
        MKT_BUY = std::numeric_limits<int>::max();;
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

void TradingAgent::kernelStarting(const Timestamp& startTime) {
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
        sendMessage(exchangeID, MarketClosePriceRequestMsg());
    }

    if (not mkt_open.isValid()) {
        // Ask our exchange when it opens and closes.
        sendMessage(exchangeID, MarketHoursRequestMsg());
    }

    /* For the sake of subclasses, TradingAgent now returns a boolean
       indicating whether the agent is "ready to trade" -- has it received
       the market open and closed times, and is the market not already closed. */
    return (mkt_open.isValid() and mkt_close.isValid()) and not mkt_closed;
}

void TradingAgent::requestDataSubscription(MarketDataSubReqMsg subscription_message) {
    subscription_message.cancel = false;

    sendMessage(exchangeID, subscription_message);
}

void TradingAgent::cancelDataSubscription(MarketDataSubReqMsg subscription_message) {
    subscription_message.cancel = true;

    sendMessage(exchangeID, subscription_message);
}

void TradingAgent::receiveMessage(Timestamp currentTime, int sender_id, Message& message) { 
    //FinancialAgent::receiveMessage(currentTime, sender_id, message); NEED TO DO

    // Do we know the market hours?
    bool had_mkt_hours = mkt_open.isValid() and mkt_close.isValid();

    // Record market open or close times.
    if (const MarketHoursMsg* marketMsg = dynamic_cast<const MarketHoursMsg*>(&message)) {
        mkt_open = marketMsg->mkt_open;
        mkt_close = marketMsg->mkt_close;

        logger->log("Recorded market open: " + mkt_open.to_string());
        logger->log("Recorded market close: " + mkt_close.to_string());
    }

    else if (const MarketClosePriceMsg* marketMsg = dynamic_cast<const MarketClosePriceMsg*>(&message)) {
        // Update the local pricing data to ensure accurate mark-to-market calculations.
        for (const auto& pair : marketMsg->close_prices) {
            const std::string& symbol = pair.first;
            int close_price = pair.second;

            last_trade[symbol] = close_price;
        }

    }
}


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

int TradingAgent::markToMarket(std::unordered_map<std::string, int>& holdings, bool use_midpoint) {
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
        logger->log("MARK_TO_MARKET" + std::to_string(shares) + symbol + " @ "  + std::to_string(last_trade[symbol]) + std::to_string(value));
    }
    return cash;
}

std::tuple<int, int, int> TradingAgent::getKnownBidAskMidpoint(std::string symbol) {
    int bid = -1;
    int ask = -1;
    int mid = -1;
    return std::make_tuple(bid, ask, mid);
}

std::tuple<int, int, int, int> TradingAgent::getKnownBidAsk(std::string symbol, bool best) {
    if (best) {
        if (known_bids.find(symbol) != known_bids.end()) {
        const auto& inner_map = known_bids[symbol];
        
        if (!inner_map.empty()) {
            // Access the most recent element using rbegin() (last element in std::map)
            auto most_recent = inner_map.rbegin();  // Reverse iterator to the last element

            return most_recent->second;
            
        } else {
            std::cout << "No bids found for " << symbol << std::endl;
            return std::make_tuple(-1,-1,-1,-1);
        }
    } else {
        std::cout << "Symbol " << symbol << " not found in known_bids" << std::endl;
        return std::make_tuple(-1,-1,-1,-1);
    }
    }
    else {
        std::cout << "ERROR NOT IMPLEMENTED NON-BEST BID/ASKS" << std::endl;
        return std::make_tuple(-1,-1,-1,-1);
    }
}

int TradingAgent::getHoldings(std::string symbol) {
    auto it = holdings.find(symbol);
    return it != holdings.end() ? it->second : 0;
}

void TradingAgent::getCurrentSpread(std::string symbol, int depth) {
    sendMessage(exchangeID, QuerySpreadMsg(symbol, depth));
}

LimitOrder TradingAgent::createLimitOrder(
    std::string symbol,
    int quantity,
    Side side,
    int limit_price,
    std::optional<int> order_id = std::nullopt,
    bool is_hidden = false,
    bool is_price_to_comply = false,
    bool insert_by_id = false,
    bool is_post_only = false,
    bool ignore_risk = true
) {
    LimitOrder order(
            id,
            currentTime,
            symbol,
            quantity,
            side,
            limit_price,
            is_hidden,
            is_price_to_comply,
            insert_by_id,
            is_post_only,
            order_id
        );
    
    if (quantity > 0) {
        // Test if this order can be permitted given our at-risk limits;
        auto new_holdings = holdings;

        int q = order.side.is_bid() ? order.quantity : -order.quantity;

        if (new_holdings.find(order.symbol) != new_holdings.end()) {
            new_holdings[order.symbol] += q;
        }
        else {
            new_holdings[order.symbol] = q;
        }

        // If at_risk is lower, always allow. Otherwise, new_at_risk must be bellow starting cash.
        if (!ignore_risk) {
            // Compute before and after at-risk capital.
            int at_risk = markToMarket(holdings) - holdings["CASH"];
            int new_at_risk = markToMarket(new_holdings) - new_holdings["CASH"];

            if (new_at_risk > at_risk && new_at_risk > starting_cash) {
                std::ostringstream oss;
                oss << "TradingAgent ignored limit order due to at-risk constraints: " << order << fmtHoldings(holdings);
                logger->log(oss.str());
                return LimitOrder();
            }
        }
        return order;
    }
    else {
        std::ostringstream oss;
        oss << "TradingAgent ignored limit order of quantity zero: " << order;
        logger->log(oss.str());
        return LimitOrder();
    }
}


void TradingAgent::placeLimitOrder(
    std::string symbol,
    int quantity,
    Side side,
    int limit_price,
    int order_id,
    bool is_hidden,
    bool is_price_to_comply,
    bool insert_by_id,
    bool is_post_only,
    bool ignore_risk
    ) {
    
    LimitOrder order = createLimitOrder(
            symbol,
            quantity,
            side,
            limit_price,
            order_id,
            is_hidden,
            is_price_to_comply,
            insert_by_id,
            is_post_only,
            ignore_risk
        );

        if (order.order_id.has_value()) {
            // Access the value of order_id and use it as the key
            orders[order.order_id.value()] = order;

            if (log_orders) {
                std::ostringstream oss;
                oss << order;
                logEvent("ORDER_SUBMITTED", oss.str());
            }
        }
}