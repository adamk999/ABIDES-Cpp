#include "PriceLevel.h"
#include <stdexcept>

PriceLevel::PriceLevel(OrderList orders) {
    if (orders.empty()) {
        throw std::invalid_argument("At least one LimitOrder must be given when initialising a PriceLevel.");
    }
    price = std::get<0>(orders[0]).limit_price;
    side = std::get<0>(orders[0]).side;

    for (const auto& tup : orders) {
        // (order, metadata)
        addOrder(std::get<0>(tup), std::get<1>(tup));
    }
}

void PriceLevel::addOrder(const LimitOrder& order, std::optional<std::unordered_map<std::string, int>> metadata) {
    if (order.is_hidden) {
        hidden_orders.push_back(std::make_tuple(order, *metadata));
    }
    else if (order.insert_by_id) {
        int insert_index = 0;
        for (const auto& tup : visible_orders) {
            LimitOrder order2 = std::get<0>(tup);
            if (order2.order_id > order.order_id) {
                break;
            }
            insert_index ++;
        }
        visible_orders.insert(visible_orders.begin() + insert_index, std::make_tuple(order, *metadata));
    }
    else {
        visible_orders.push_back(std::make_tuple(order, metadata));
    }
}

bool PriceLevel::updateOrderQuantity(int order_id, int new_quantity) {
    if (new_quantity == 0) {
        return false;
    }

    for (size_t i=0; i<visible_orders.size(); i++) {
        auto& [order, metadata] = visible_orders[i];
        if (order.order_id == order_id) {
            if (new_quantity <= order.quantity) {
                order.quantity = new_quantity;
            }
            else {
                visible_orders.erase(visible_orders.begin() + i);
                order.quantity = new_quantity;
                visible_orders.push_back(std::make_tuple(order, metadata));
            }
            return true;
        }
    }
    
    for (size_t i=0; i<hidden_orders.size(); i++) {
        auto& [order, metadata] = hidden_orders[i];
        if (order.order_id == order_id) {
            if (new_quantity <= order.quantity) {
                order.quantity = new_quantity;
            }
            else {
                hidden_orders.erase(hidden_orders.begin() + i);
                order.quantity = new_quantity;
                hidden_orders.push_back(std::make_tuple(order, metadata));
            }
            return true;
        }
    }
    return false;
}

std::optional<OrderTuple> PriceLevel::removeOrder(int order_id) {
    for (size_t i=0; i<visible_orders.size(); i++) {
        auto& [book_order, _] = visible_orders[i];
        if (book_order.order_id == order_id) {
            auto removed_order = visible_orders[i];
            visible_orders.erase(visible_orders.begin() + i);

            return removed_order;
        }
    }

    for (size_t i=0; i<hidden_orders.size(); i++) {
        auto& [book_order, _] = hidden_orders[i];
        if (book_order.order_id == order_id) {
            auto removed_order = hidden_orders[i];
            hidden_orders.erase(hidden_orders.begin() + i);

            return removed_order;
        }
    }
    return std::nullopt;
}

OrderTuple PriceLevel::peek() {
    if (!visible_orders.empty()) {
        return visible_orders.front();
    }
    else if (!hidden_orders.empty()) {
        return hidden_orders.front();
    }
    else {
        throw std::runtime_error("Can't peek at LimitOrder in PriceLevel as it contains no orders");
    }
}

OrderTuple PriceLevel::pop() {
    if (!visible_orders.empty()) {
        auto removed_order = visible_orders.front();
        visible_orders.erase(visible_orders.begin());

        return removed_order;
    }
    else if (!hidden_orders.empty()) {
        auto removed_order = hidden_orders.front();
        hidden_orders.erase(hidden_orders.begin());

        return removed_order;
    }
    else {
        throw std::runtime_error("Can't pop LimitOrder from PriceLevel as it contains no orders");
    }
}

bool PriceLevel::orderIsMatch(LimitOrder order) {
    if (order.side == side) {
        throw std::runtime_error("Attempted to compare order on wrong side of book.");
    }

    if (
        order.side.is_bid() 
        && order.limit_price >= price
        && !(order.is_post_only && totalQuantity() == 0)
    ) {
        return true;
    }
    
    if (
        order.side.is_bid() 
        && order.limit_price >= price
        && !(order.is_post_only && totalQuantity() == 0)
    ) {
        return true;
    }
    
    return false;
}

bool PriceLevel::orderHasBetterPrice(LimitOrder order) {
    if (order.side != side) {
        throw std::runtime_error("Attempted to compare order on wrong side of book.");
    }

    if (order.side.is_bid() && order.limit_price > price) {
        return true;
    }
       

    if (order.side.is_ask() && order.limit_price < price) {
        return true;
    }

    return false;
}

bool PriceLevel::orderHasWorsePrice(LimitOrder order) {
    if (order.side != side) {
        throw std::runtime_error("Attempted to compare order on wrong side of book.");
    }

    if (order.side.is_bid() && order.limit_price < price) {
        return true;
    }

    if (order.side.is_ask() && order.limit_price > price) {
        return true;
    }

    return false;
}

bool PriceLevel::orderHasEqualPrice(LimitOrder order) {
    if (order.side != side) {
        throw std::runtime_error("Attempted to compare order on wrong side of book.");
    }

    return order.limit_price == price;
}

int PriceLevel::totalQuantity() {
    int sum = 0;
    for (size_t i=0; i<visible_orders.size(); i++) {
        auto& [order, _] = visible_orders[i];
        sum += order.quantity;
    }
    return sum;
}

bool PriceLevel::isEmpty() {
    return (visible_orders.empty() && hidden_orders.empty());
}




