#pragma once
#include <optional>
#include "FinancialAgent.h"
#include <vector>

struct Side;

class ExchangeAgent : public FinancialAgent {
    /*
    The ExchangeAgent expects a numeric agent id, printable name, agent type, timestamp
    to open and close trading, a list of equity symbols for which it should create order
    books, a frequency at which to archive snapshots of its order books, a pipeline
    delay (in ns) for order activity, the exchange computation delay (in ns), the levels
    of order stream history to maintain per symbol (maintains all orders that led to the
    last N trades), whether to log all order activity to the agent log, and a random
    state object (already seeded) to use for stochasticity.
    */

    struct MetricTracker {
        // droupout metrics
        int total_time_no_liquidity_asks;
        int total_time_no_liquidity_bids;
        float pct_time_no_liquidity_asks;
        float pct_time_no_liquidity_bids;

        // exchanged volume
        int total_exchanged_volume;

        // last trade
        std::optional<int> last_trade;
        // can be extended

        MetricTracker() {
            total_time_no_liquidity_asks = 0;
            total_time_no_liquidity_bids = 0;
            pct_time_no_liquidity_asks = 0;
            pct_time_no_liquidity_bids = 0;
            total_exchanged_volume = 0;
            last_trade = 0;
        }
    };

    struct BaseDataSubscription {
        /*
        Base class for all types of data subscription registered with this agent.
        */

        int agent_id;
        Timestamp last_update_ts;

        BaseDataSubscription(int agent_id, Timestamp last_update_ts) 
        : agent_id(agent_id), last_update_ts(last_update_ts) {}
    };


    struct FrequencyBasedSubscription : public BaseDataSubscription {
        /*
        Base class for all types of data subscription that are sent from this agent
        at a fixed, regular frequency.
        */
        int freq;

        FrequencyBasedSubscription(int agent_id, Timestamp last_update_ts, int freq) 
        : BaseDataSubscription(agent_id, last_update_ts), freq(freq) {}
    };


    struct L1DataSubscription : public FrequencyBasedSubscription {
        L1DataSubscription(int agent_id, Timestamp last_update_ts, int freq)
        : FrequencyBasedSubscription(agent_id, last_update_ts, freq) {}
    };


    struct L2DataSubscription : FrequencyBasedSubscription {
        int depth;

        L2DataSubscription(int agent_id, Timestamp last_update_ts, int freq, int depth)
        : FrequencyBasedSubscription(agent_id, last_update_ts, freq), depth(depth) {}
    };


    struct L3DataSubscription : public FrequencyBasedSubscription {
        int depth;

        L3DataSubscription(int agent_id, Timestamp last_update_ts, int freq, int depth)
        : FrequencyBasedSubscription(agent_id, last_update_ts, freq), depth(depth) {}
    };


    struct TransactedVolDataSubscription: public FrequencyBasedSubscription {
        std::string lookback;

        TransactedVolDataSubscription(int agent_id, Timestamp last_update_ts, int freq, int depth, std::string lookback)
        : FrequencyBasedSubscription(agent_id, last_update_ts, freq), lookback(lookback) {}
    };


    struct EventBasedSubscription : public BaseDataSubscription {
        /*
        Base class for all types of data subscription that are sent from this agent
        when triggered by an event or specific circumstance.
        */
        bool event_in_progress;

        EventBasedSubscription(int agent_id, Timestamp last_update_ts, bool event_in_progress)
        : BaseDataSubscription(agent_id, last_update_ts), event_in_progress(event_in_progress) {}
    };


    struct BookImbalanceDataSubscription : public EventBasedSubscription {
        // Properties:
        float min_imbalance;

        // State:
        std::optional<float> imbalance;
        std::optional<Side> side;

        BookImbalanceDataSubscription(
            int agent_id, 
            Timestamp last_update_ts, 
            bool event_in_progress,
            float min_imbalance, 
            std::optional<float> imbalance = std::nullopt,
            std::optional<Side> side = std::nullopt
            ) : EventBasedSubscription(agent_id, last_update_ts, event_in_progress),
                min_imbalance(min_imbalance), imbalance(imbalance), side(side) {}
    };

    bool reschedule;
    std::vector<std::string> symbols;
    Timestamp mkt_close;
    int pipeline_delay;
    int computational_delay;
    int book_log_depth;
    bool book_logging;
    bool log_orders;
    int stream_history;

public:
    Timestamp mkt_open;

    ExchangeAgent(
        int id, 
        Timestamp mkt_open,
        Timestamp mkt_close,
        std::vector<std::string> symbols,
        Logger& logger,
        std::optional<std::string> name = std::nullopt,
        std::optional<std::string> type = std::nullopt,
        bool book_logging = true,
        int book_log_depth = 10,
        int pipeline_delay = 40000,
        int computational_delay = 1,
        int stream_history = 0,
        bool log_orders = false,
        int random_state = -1,
        bool use_metric_tracker = true
        );
};