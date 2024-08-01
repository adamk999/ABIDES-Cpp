#include "FinancialAgent.h"
#include <cmath>
#include <unordered_map>
#include "../util/timestamping.h"

/* The TradingAgent class (via FinancialAgent, via Agent) is intended as the
   base class for all trading agents (i.e. not things like exchanges) in a
   market simulation.  It handles a lot of messaging (inbound and outbound)
   and state maintenance automatically, so subclasses can focus just on
   implementing a strategy without too much bookkeeping. */

class TradingAgent : public FinancialAgent {

private:
    // Not yet aware of when the exchange opens/closes.
    Timestamp mkt_open;
    Timestamp mkt_close;

    /* Note that agents are limited by their starting cash, currently
    without leverage.  Taking short positions is permitted,
    but does NOT increase the amount of at-risk capital allowed. */
    const int starting_cash;
    int cash;
    int exchangeID;
    bool log_orders;
    bool first_wake;
    bool mkt_closed;
    int MKT_BUY;
    int MKT_SELL;
                
    int nav_diff;
    int basket_size;

    std::unordered_map<std::string, int> holdings;
    std::unordered_map<int, int> orders;

    /* The base TradingAgent also tracks last known prices for every symbol
        for which it has received as QUERY_LAST_TRADE message.  Subclass
        agents may use or ignore this as they wish.  Note that the subclass
        agent must request pricing when it wants it.  This agent does NOT
        automatically generate such requests, though it has a helper function
        that can be used to make it happen. */
    std::unordered_map<std::string, int> last_trade;

    // Used in subscription mode to record the timestamp for which the data was current in the ExchangeAgent.
    std::unordered_map<std::string, Timestamp> exchange_ts;

    /* When a last trade price comes in after market close, the trading agent
       automatically records it as the daily close price for a symbol.*/
    std::unordered_map<std::string, int> daily_close_price;

    /* The agent remembers the last known bids and asks (with variable depth,
       showing only aggregate volume at each price level) when it receives
       a response to QUERY_SPREAD. */
    std::unordered_map<std::string, int> known_bids;
    std::unordered_map<std::string, int> known_asks;

    /* The agent remembers the order history communicated by the exchange
       when such is requested by an agent (for example, a heuristic belief
       learning agent). */
    std::unordered_map<std::string, int> stream_history;

    // The agent records the total transacted volume in the exchange for a given symbol and lookback period.
    std::unordered_map<std::string, int> transacted_volume;

    // Each agent can choose to log the orders executed.
    std::vector<std::unordered_map<std::string, int> > executed_orders;

public:
    TradingAgent(
        int id, 
        std::string name, 
        std::string type, 
        int random_state, 
        const int starting_cash = 100000, 
        Logger& logger,
        bool log_orders = false,
        bool log_to_file = true
        );

    void kernelStarting(const int& startTime);

    void kernelStopping();

    bool wakeup(const Timestamp currentTime);

    void requestDataSubscription(std::string symbol, const int levels, const int freq);

    // Used by any Trading Agent subclass to cancel subscription to market data from the Exchange Agent
    void cancelDataSubscription(std::string symbol);

    std::string fmtHoldings(const std::unordered_map<std::string, int>& holdings);

    int markToMarket(std::unordered_map<std::string, int>& holdings, bool use_midpoint = false);

    std::tuple<int, int, int> getKnownBidAskMidpoint(std::string symbol);


};