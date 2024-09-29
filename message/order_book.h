#include "Message.h"
#include "orders.h"

struct OrderBookMsg : public Message {
    virtual ~OrderBookMsg() = default;
};

struct OrderAcceptedMsg : public OrderBookMsg {
    LimitOrder order;
    OrderAcceptedMsg(const LimitOrder& order) : order(order) {}
};


struct OrderExecutedMsg : public OrderBookMsg {
    Order order;
    OrderExecutedMsg(const Order& order) : order(order) {}
};


struct OrderCancelledMsg : public OrderBookMsg {
    LimitOrder order;
    OrderCancelledMsg(const LimitOrder& order) : order(order) {}
};


struct OrderPartialCancelledMsg : public OrderBookMsg {
    LimitOrder new_order;
    OrderPartialCancelledMsg(const LimitOrder& new_order) : new_order(new_order) {}
};


struct OrderModifiedMsg : public OrderBookMsg {
    LimitOrder new_order;
    OrderModifiedMsg(const LimitOrder& new_order) : new_order(new_order) {}
};


struct OrderReplacedMsg : public OrderBookMsg {
    LimitOrder old_order;
    LimitOrder new_order;
    OrderReplacedMsg(const LimitOrder& old_order, const LimitOrder& new_order) 
    : old_order(old_order), new_order(new_order) {}
};
