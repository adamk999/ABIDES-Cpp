#pragma once
#include "Message.h"
#include "../util/timestamping.h"
#include <vector>
#include <tuple>
#include <limits>
#include <array>
//from ..orders import Side


class MarketDataSubReqMsg : public Message {
    /*
    Base class for creating or cancelling market data subscriptions with an
    ``ExchangeAgent``.

    Attributes:
        symbol: The symbol of the security to request a data subscription for.
        cancel: If True attempts to create a new subscription, if False attempts to
            cancel an existing subscription.
    */

public:
    std::string symbol;
    bool cancel;

    MarketDataSubReqMsg(std::string symbol, bool cancel) : Message(), symbol(symbol), cancel(cancel) {}
};


class MarketDataFreqBasedSubReqMsg : public MarketDataSubReqMsg {   
    /*
    Base class for creating or cancelling market data subscriptions with an
    ``ExchangeAgent``.

    Attributes:
        symbol: The symbol of the security to request a data subscription for.
        cancel: If True attempts to create a new subscription, if False attempts to
            cancel an existing subscription.
        freq: The frequency in nanoseconds^-1 at which to receive market updates.
    */

    // Inherited Fields:
    // symbol: str
    // cancel: bool = False
    int freq;
    MarketDataFreqBasedSubReqMsg(std::string symbol, bool cancel, int freq=1) : MarketDataSubReqMsg(symbol, cancel), freq(freq) {}
};


class MarketDataEventBasedSubReqMsg : public MarketDataSubReqMsg {
    /*
    Base class for creating or cancelling market data subscriptions with an
    ``ExchangeAgent``.

    Attributes:
        symbol: The symbol of the security to request a data subscription for.
        cancel: If True attempts to create a new subscription, if False attempts to
            cancel an existing subscription.
    */

    // Inherited Fields:
    // symbol: str
    // cancel: bool = False
};



class L1SubReqMsg : public MarketDataFreqBasedSubReqMsg {
    /*
    This message requests the creation or cancellation of a subscription to L1 order
    book data from an ``ExchangeAgent``.

    Attributes:
        symbol: The symbol of the security to request a data subscription for.
        cancel: If True attempts to create a new subscription, if False attempts to
            cancel an existing subscription.
        freq: The frequency in nanoseconds^-1 at which to receive market updates.
    */

    // Inherited Fields:
    // symbol: str
    // cancel: bool = False
    // freq: int = 1
};


class L2SubReqMsg : public MarketDataFreqBasedSubReqMsg {
    /*
    This message requests the creation or cancellation of a subscription to L2 order
    book data from an ``ExchangeAgent``.

    Attributes:
        symbol: The symbol of the security to request a data subscription for.
        cancel: If True attempts to create a new subscription, if False attempts to
            cancel an existing subscription.
        freq: The frequency in nanoseconds^-1 at which to receive market updates.
        depth: The maximum number of price levels on both sides of the order book to
            return data for. Defaults to the entire book.
    */

    // Inherited Fields:
    // symbol: str
    // cancel: bool = False
    // freq: int = 1
    int depth = std::numeric_limits<int>::max();
};


class L3SubReqMsg : public MarketDataFreqBasedSubReqMsg {
    /*
    This message requests the creation or cancellation of a subscription to L3 order
    book data from an ``ExchangeAgent``.

    Attributes:
        symbol: The symbol of the security to request a data subscription for.
        cancel: If True attempts to create a new subscription, if False attempts to
            cancel an existing subscription.
        freq: The frequency in nanoseconds^-1 at which to receive market updates.
        depth: The maximum number of price levels on both sides of the order book to
            return data for. Defaults to the entire book.
    */

    // Inherited Fields:
    // symbol: str
    // cancel: bool = False
    // freq: int = 1
    int depth = std::numeric_limits<int>::max();
};


class TransactedVolSubReqMsg : public MarketDataFreqBasedSubReqMsg {
    /*      
    This message requests the creation or cancellation of a subscription to transacted
    volume order book data from an ``ExchangeAgent``.

    Attributes:
        symbol: The symbol of the security to request a data subscription for.
        cancel: If True attempts to create a new subscription, if False attempts to
            cancel an existing subscription.
        freq: The frequency in nanoseconds^-1 at which to receive market updates.
        lookback: The period in time backwards from the present to sum the transacted
            volume for.
    */

    // Inherited Fields:
    // symbol: str
    // cancel: bool = False
    // freq: int = 1
    std::string lookback = "1min";
};


class BookImbalanceSubReqMsg : public MarketDataEventBasedSubReqMsg {
    /*
    This message requests the creation or cancellation of a subscription to book
    imbalance events.

    Attributes:
        symbol: The symbol of the security to request a data subscription for.
        cancel: If True attempts to create a new subscription, if False attempts to
            cancel an existing subscription.
        min_imbalance: The minimum book imbalance needed to trigger this subscription.

    0.0 is no imbalance.
    1.0 is full imbalance (ie. liquidity drop).
    */

    // Inherited Fields:
    // symbol: str
    // cancel: bool = False
    float min_imbalance = 1.0;
};

class MarketDataMsg : public Message {  
    /*
    Base class for returning market data subscription results from an ``ExchangeAgent``.

    The ``last_transaction`` and ``exchange_ts`` fields are not directly related to the
    subscription data but are included for bookkeeping purposes.

    Attributes:
        symbol: The symbol of the security this data is for.
        last_transaction: The time of the last transaction that happened on the exchange.
        exchange_ts: The time that the message was sent from the exchange.
    */

    std::string symbol;
    int last_transaction;
    Timestamp exchange_ts;
};


class MarketDataEventMsg : public MarketDataMsg {
    /*
    Base class for returning market data subscription results from an ``ExchangeAgent``.

    The ``last_transaction`` and ``exchange_ts`` fields are not directly related to the
    subscription data but are included for bookkeeping purposes.

    Attributes:
        symbol: The symbol of the security this data is for.
        last_transaction: The time of the last transaction that happened on the exchange.
        exchange_ts: The time that the message was sent from the exchange.
        stage: The stage of this event (start or finish).
    */

    enum class Stage {
        START,
        FINISH
    };

    Stage stage;
};


class L1DataMsg : MarketDataMsg {
    /*
    This message returns L1 order book data as part of an L1 data subscription.

    Attributes:
        symbol: The symbol of the security this data is for.
        last_transaction: The time of the last transaction that happened on the exchange.
        exchange_ts: The time that the message was sent from the exchange.
        bid: The best bid price and the available volume at that price.
        ask: The best ask price and the available volume at that price.
    */

    // Inherited Fields:
    // symbol: str
    // last_transaction: int
    // exchange_ts: NanosecondTime
    int bid[2];
    int ask[2];
};


class L2DataMsg : public MarketDataMsg {
    /*
    This message returns L2 order book data as part of an L2 data subscription.

    Attributes:
        symbol: The symbol of the security this data is for.
        last_transaction: The time of the last transaction that happened on the exchange.
        exchange_ts: The time that the message was sent from the exchange.
        bids: A list of tuples containing the price and available volume at each bid
            price level.
        asks: A list of tuples containing the price and available volume at each ask
            price level.
    */

    // Inherited Fields:
    // symbol: str
    // last_transaction: int
    // exchange_ts: NanosecondTime

    std::vector<std::array<int, 2>> bids;
    std::vector<std::array<int, 2>> asks;

    // TODO: include requested depth
};


class L3DataMsg : MarketDataMsg {
    /*
    This message returns L3 order book data as part of an L3 data subscription.

    Attributes:
        symbol: The symbol of the security this data is for.
        last_transaction: The time of the last transaction that happened on the exchange.
        exchange_ts: The time that the message was sent from the exchange.
        bids: A list of tuples containing the price and a list of order sizes at each
            bid price level.
        asks: A list of tuples containing the price and a list of order sizes at each
            ask price level.
    */

    // Inherited Fields:
    // symbol: str
    // last_transaction: int
    // exchange_ts: NanosecondTime
    std::vector<std::tuple<int, std::vector<int>>> bids;
    std::vector<std::tuple<int, std::vector<int>>> asks;

    // TODO: include requested depth
};


class TransactedVolDataMsg : public MarketDataMsg {
    /*
    This message returns order book transacted volume data as part of an transacted
    volume data subscription.

    Attributes:
        symbol: The symbol of the security this data is for.
        last_transaction: The time of the last transaction that happened on the exchange.
        exchange_ts: The time that the message was sent from the exchange.
        bid_volume: The total transacted volume of bid orders for the given lookback period.
        ask_volume: The total transacted volume of ask orders for the given lookback period.
    */

    // Inherited Fields:
    // symbol: str
    // last_transaction: int
    // exchange_ts: NanosecondTime
    int bid_volume;
    int ask_volume;

    // TODO: include lookback period
};


class BookImbalanceDataMsg : public MarketDataEventMsg {    
    /*
    Sent when the book imbalance reaches a certain threshold dictated in the
    subscription request message.

    Attributes:
        symbol: The symbol of the security this data is for.
        last_transaction: The time of the last transaction that happened on the exchange.
        exchange_ts: The time that the message was sent from the exchange.
        stage: The stage of this event (start or finish).
        imbalance: Proportional size of the imbalance.
        side: Side of the book that the imbalance is towards.
    */

    // Inherited Fields:
    // symbol: str
    // last_transaction: int
    // exchange_ts: pd.Timestamp
    // stage: MarketDataEventMsg.Stage
    float imbalance;
    std::string side;
};