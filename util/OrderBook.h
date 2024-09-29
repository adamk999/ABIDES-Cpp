#pragma once
#include <set>
#include <string>
#include "../agents/ExchangeAgent.h"
#include "PriceLevel.h"

class OrderBook {
    /*
    Basic class for an order book for one symbol, in the style of the major US Stock Exchanges.

    An OrderBook requires an owning agent object, which it will use to send messages
    outbound via the simulator Kernel (notifications of order creation, rejection,
    cancellation, execution, etc).

    Attributes:
        owner: The agent this order book belongs to.
        symbol: The symbol of the stock or security that is traded on this order book.
        bids: List of bid price levels (index zero is best bid), stored as a PriceLevel object.
        asks: List of ask price levels (index zero is best ask), stored as a PriceLevel object.
        last_trade: The price that the last trade was made at.
        book_log: Log of the full order book depth (price and volume) each time it changes.
        book_log2: TODO
        quotes_seen: TODO
        history: A truncated history of previous trades.
        last_update_ts: The last timestamp the order book was updated.
        buy_transactions: An ordered list of all previous buy transaction timestamps and quantities.
        sell_transactions: An ordered list of all previous sell transaction timestamps and quantities.
    */

    ExchangeAgent owner;
    std::string symbol;

    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
    int last_trade;

    // Create an empty list of dictionaries to log the full order book depth (price and volume) each time it changes.
    std::vector<std::unordered_map<std::string, int>> book_log2;
    std::set<int> quotes_seen;  

    // Create an order history for the exchange to report to certain agent types.
    std::vector<std::unordered_map<std::string, int>> history;

    Timestamp last_update_ts;
    std::vector<std::tuple<Timestamp, int>> buy_transactions;
    std::vector<std::tuple<Timestamp, int>> sell_transactions;

public:
    OrderBook(ExchangeAgent owner, std::string symbol);
        /*
        Creates a new OrderBook class instance for a single symbol.

        Arguments:
            owner: The agent this order book belongs to, usually an `ExchangeAgent`.
            symbol: The symbol of the stock or security that is traded on this order book.
        */

    void handleLimitOrder(LimitOrder order, bool quiet = false);
        /*
        Matches a limit order or adds it to the order book.

        Handles partial matches piecewise,
        consuming all possible shares at the best price before moving on, without regard to
        order size "fit" or minimizing number of transactions.  Sends one notification per
        match.

        Arguments:
            order: The limit order to process.
            quiet: If True messages will not be sent to agents and entries will not be added to
                history. Used when this function is a part of a more complex order.
        */

    void handleMarketOrder(const MarketOrder& order);
        /*
        Takes a market order and attempts to fill at the current best market price.

        Arguments:
            order: The market order to process.
        */

    std::optional<Order> executeOrder(Order order);
        /*
        Finds a single best match for this order, without regard for quantity.

        Returns the matched order or None if no match found.  DOES remove,
        or decrement quantity from, the matched order from the order book
        (i.e. executes at least a partial trade, if possible).

        Arguments:
            order: The order to execute.
        */

    
       
};