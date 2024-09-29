#pragma once
#include "../util/timestamping.h"
#include "FinancialAgent.h"
#include <cmath>
#include <unordered_map>
#include <map>
#include "../message/market_data.h"
#include <optional>

struct LimitOrder;
class Side;
struct Order;

/* The TradingAgent class (via FinancialAgent, via Agent) is intended as the
   base class for all trading agents (i.e. not things like exchanges) in a
   market simulation.  It handles a lot of messaging (inbound and outbound)
   and state maintenance automatically, so subclasses can focus just on
   implementing a strategy without too much bookkeeping. */

class TradingAgent : public FinancialAgent {

protected:
    /* Note that agents are limited by their starting cash, currently
    without leverage.  Taking short positions is permitted,
    but does NOT increase the amount of at-risk capital allowed. */
    int cash;
    bool log_orders;
    bool first_wake;
    int MKT_BUY;
    int MKT_SELL;
    int exchangeID;
                
    int nav_diff;
    int basket_size;

    std::unordered_map<int, Order> orders;

    // Used in subscription mode to record the timestamp for which the data was current in the ExchangeAgent.
    std::unordered_map<std::string, Timestamp> exchange_ts;

    /* The agent remembers the last known bids and asks (with variable depth,
       showing only aggregate volume at each price level) when it receives
       a response to QUERY_SPREAD. */
    std::unordered_map<std::string, std::map<Timestamp, std::tuple<int, int, int, int>>> known_bids;
    std::unordered_map<std::string, std::map<Timestamp, std::tuple<int, int, int, int>>> known_asks;

    /* The agent remembers the order history communicated by the exchange
       when such is requested by an agent (for example, a heuristic belief
       learning agent). */
    std::unordered_map<std::string, int> stream_history;

    // The agent records the total transacted volume in the exchange for a given symbol and lookback period.
    std::unordered_map<std::string, int> transacted_volume;

    // Each agent can choose to log the orders executed.
    std::vector<std::unordered_map<std::string, int>> executed_orders;

public:
    const int starting_cash;
    std::unordered_map<std::string, int> holdings;
    bool mkt_closed;
    
    // Not yet aware of when the exchange opens/closes.
    Timestamp mkt_open;
    Timestamp mkt_close;
    
    /*  The base TradingAgent also tracks last known prices for every symbol
        for which it has received as QUERY_LAST_TRADE message.  Subclass
        agents may use or ignore this as they wish.  Note that the subclass
        agent must request pricing when it wants it.  This agent does NOT
        automatically generate such requests, though it has a helper function
        that can be used to make it happen. */
    std::unordered_map<std::string, int> last_trade;

    /* When a last trade price comes in after market close, the trading agent
       automatically records it as the daily close price for a symbol.*/
    std::unordered_map<std::string, int> daily_close_price;

    TradingAgent(
        int id, 
        std::string name, 
        std::string type, 
        int random_state, 
        Logger& logger,
        const int starting_cash = 100000, 
        bool log_orders = false,
        bool log_to_file = true
        );

    void kernelStarting(const Timestamp& startTime);

    void kernelStopping();

    bool wakeup(const Timestamp currentTime);

    void requestDataSubscription(MarketDataSubReqMsg subscription_message);
    /*
        Used by any Trading Agent subclass to create a subscription to market data from
        the Exchange Agent.

        Arguments:
            subscription_message: An instance of a MarketDataSubReqMessage.
    */

    void cancelDataSubscription(MarketDataSubReqMsg subscription_message);
    /*
        Used by any Trading Agent subclass to cancel subscription to market data from
        the Exchange Agent.

        Arguments:
            subscription_message: An instance of a MarketDataSubReqMessage.
    */

    void receiveMessage(Timestamp currentTime, int sender_id, Message& message);
    /*
        Arguments:
            current_time: The time that this agent received the message.
            sender_id: The ID of the agent who sent the message.
            message: The message contents.
    */

    std::string fmtHoldings(const std::unordered_map<std::string, int>& holdings);

    int markToMarket(std::unordered_map<std::string, int>& holdings, bool use_midpoint = false);

    std::tuple<int, int, int> getKnownBidAskMidpoint(std::string symbol);

    std::tuple<int, int, int, int> getKnownBidAsk(std::string symbol, bool best = true);
    /*
        Extract the current known bid and asks.

        This does NOT request new information.

        Arguments:
            symbol: The symbol to query.
            best:
    */

   int getHoldings(std::string symbol);
    /*
        Gets holdings.  Returns zero for any symbol not held.

        Arguments:
            symbol: The symbol to query.
    */

    void getCurrentSpread(std::string symbol, int depth = 1);
    /*
        Used by any Trading Agent subclass to query the current spread for a symbol.

        This activity is not logged.

        Arguments:
            symbol: The symbol to query.
            depth:
    */

    LimitOrder createLimitOrder(
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
    );
        /*
        Used by any Trading Agent subclass to create a limit order.

        Arguments:
            symbol: A valid symbol.
            quantity: Positive share quantity.
            side: Side.BID or Side.ASK.
            limit_price: Price in cents.
            order_id: An optional order id (otherwise global autoincrement is used).
            is_hidden:
            is_price_to_comply:
            insert_by_id:
            is_post_only:
            ignore_risk: Whether cash or risk limits should be enforced or ignored for
                the order.
        */

   void placeLimitOrder(
        std::string symbol,
        int quantity,
        Side side,
        int limit_price,
        int order_id = -1,
        bool is_hidden = false,
        bool is_price_to_comply = false,
        bool insert_by_id = false,
        bool is_post_only = false,
        bool ignore_risk = true
    );
    /*
        Used by any Trading Agent subclass to place a limit order.

        Arguments:
            symbol: A valid symbol.
            quantity: Positive share quantity.
            side: Side.BID or Side.ASK.
            limit_price: Price in cents.
            order_id: An optional order id (otherwise global autoincrement is used).
            is_hidden:
            is_price_to_comply:
            insert_by_id:
            is_post_only:
            ignore_risk: Whether cash or risk limits should be enforced or ignored for
                the order.
    */

};