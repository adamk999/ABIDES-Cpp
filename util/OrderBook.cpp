#include "OrderBook.h"
#include "../message/order_book.h"

OrderBook::OrderBook(ExchangeAgent owner, std::string symbol) 
    : owner(owner), symbol(symbol) {
    last_update_ts = owner.mkt_open;
}

void OrderBook::handleLimitOrder(LimitOrder order, bool quiet) {
    if (order.symbol != symbol) {
        owner.logger->log(order.symbol + " order discarded. Does not match OrderBook symbol: " + symbol);
        return;
    }

    if (order.quantity <= 0 || int(order.quantity) != order.quantity) {
        owner.logger->log(order.symbol + " order discarded.Quantity (" + std::to_string(order.quantity) + ") must be a positive integer.");
        return;
    }

    if (order.limit_price < 0 || int(order.limit_price != order.limit_price)) {
        owner.logger->log(order.symbol + " order discarded. Limit price (" + std::to_string(order.limit_price) + ") must be a positive integer.");
        return;
    }

    std::vector<std::tuple<int,int>> executed;

    while (true) {
        std::optional<Order> matched_order = executeOrder(order);

        if (matched_order.has_value()) {
            // Accumulate the volume and average share price of the currently executing inbound trade.
            assert(matched_order.value().fill_price != -1);

            executed.push_back(std::make_tuple(matched_order.value().quantity, matched_order.value().fill_price));

            if (order.quantity <= 0) {
                break;
            }
        }
        else {
            // No matching order was found, so the new order enters the order book. Notify the agent.
            enterOrder(order, quiet=quiet);

            std::ostringstream oss;
            oss << "ACCEPTED: new order " << order;
            owner.logger->log(oss.str());

            owner.logger->log("SENT: notifications of order acceptance to agent " + str(order.agentID) 
                              + " for order " + str(order.order_id));

            if (!quiet) {
                owner.sendMessage(order.agentID, OrderAcceptedMsg(order));
            }

            break;
        }
    }

    // Now that we are done executing or accepting this order, log the new best bid and ask.
    if (!bids.empty()) {
        std::ostringstream oss;
        oss << symbol << ", " << bids[0].price << ", " << bids[0].totalQuantity();
        owner.logEvent("BEST_BID", oss.str());
    }

    if (!asks.empty()) {
        std::ostringstream oss;
        oss << symbol << ", " << asks[0].price << ", " << asks[0].totalQuantity();
        owner.logEvent("BEST_ASK", oss.str());
    }

    // Also log the last trade (total share quantity, average share price).
    if (!executed.empty()) {
        int trade_qty = 0;
        int trade_price = 0;

        for (const auto& [q, p] : executed) {
            owner.logger->log("Executed: " + str(q) + " @ " + str(p));
            trade_qty += q;
            trade_price += p*q;
        }

        int avg_price = int(round(trade_price/trade_qty));
        owner.logger->log("Avg: " + str(trade_qty) + " @ $" + str(avg_price));

        last_trade = avg_price;
    }
}

void OrderBook::handleMarketOrder(const MarketOrder& order) {

    if (order.symbol != symbol) {
        owner.logger->log(order.symbol + " order discarded. Does not match OrderBook symbol: " + symbol);
        return;
    }

    if (order.quantity <= 0 || int(order.quantity) != order.quantity) {
        owner.logger->log(symbol + " order discarded. Quantity (" + str(order.quantity) + ") must be a positive integer.");
        return;
    }

    while (order.quantity > 0) {
        if (executeOrder(order) == std::nullopt) {
            break;
        }
    }
}

std::optional<Order> OrderBook::executeOrder(Order order) {
    // Track which (if any) existing order was matched with the current order.
    std::vector<PriceLevel> book = order.side.is_bid() ? asks : bids;

    // First, examine the correct side of the order book for a match.
    if (book.empty()) {
        // No orders on this side.
        return std::nullopt;
    }

    else if (order.getName() == )

    
    

        # First, examine the correct side of the order book for a match.
        if len(book) == 0:
            # No orders on this side.
            return None
        elif isinstance(order, LimitOrder) and not book[0].order_is_match(order):
            # There were orders on the right side, but the prices do not overlap.
            # Or: bid could not match with best ask, or vice versa.
            # Or: bid offer is below the lowest asking price, or vice versa.
            return None
        elif order.tag in ["MR_preprocess_ADD", "MR_preprocess_REPLACE"]:
            # if an order enters here it means it was going to execute at entry
            # but instead it was caught by MR_preprocess_add
            self.owner.logEvent(order.tag + "_POST_ONLY", {"order_id": order.order_id})
            return None
        else:
            # There are orders on the right side, and the new order's price does fall
            # somewhere within them.  We can/will only match against the oldest order
            # among those with the best price.  (i.e. best price, then FIFO)

            # The matched order might be only partially filled. (i.e. new order is smaller)
            is_ptc_exec = False
            if order.quantity >= book[0].peek()[0].quantity:
                # Consume entire matched order.
                matched_order, matched_order_metadata = book[0].pop()

                # If the order is a part of a price to comply pair, also remove the other
                # half of the order from the book.
                if matched_order.is_price_to_comply:
                    is_ptc_exec = True
                    if matched_order_metadata["ptc_hidden"] == False:
                        raise Exception(
                            "Should not be executing on the visible half of a price to comply order!"
                        )

                    assert book[1].remove_order(matched_order.order_id) is not None

                    if book[1].is_empty:
                        del book[1]

                # If the matched price now has no orders, remove it completely.
                if book[0].is_empty:
                    del book[0]
            else:
                # Consume only part of matched order.
                book_order, book_order_metadata = book[0].peek()

                matched_order = deepcopy(book_order)
                matched_order.quantity = order.quantity

                book_order.quantity -= matched_order.quantity

                # If the order is a part of a price to comply pair, also adjust the
                # quantity of the other half of the pair.
                if book_order.is_price_to_comply:
                    is_ptc_exec = True
                    if book_order_metadata["ptc_hidden"] == False:
                        raise Exception(
                            "Should not be executing on the visible half of a price to comply order!"
                        )

                    book_order_metadata[
                        "ptc_other_half"
                    ].quantity -= matched_order.quantity

            # When two limit orders are matched, they execute at the price that
            # was being "advertised" in the order book.
            matched_order.fill_price = matched_order.limit_price

            if order.side.is_bid():
                self.buy_transactions.append(
                    (self.owner.current_time, matched_order.quantity)
                )
            else:
                self.sell_transactions.append(
                    (self.owner.current_time, matched_order.quantity)
                )

            self.history.append(
                dict(
                    time=self.owner.current_time,
                    type="EXEC",
                    order_id=matched_order.order_id,
                    agent_id=matched_order.agent_id,
                    oppos_order_id=order.order_id,
                    oppos_agent_id=order.agent_id,
                    side="SELL"
                    if order.side.is_bid()
                    else "BUY",  # by def exec if from point of view of passive order being exec
                    quantity=matched_order.quantity,
                    price=matched_order.limit_price if is_ptc_exec else None,
                )
            )

            filled_order = deepcopy(order)
            filled_order.quantity = matched_order.quantity
            filled_order.fill_price = matched_order.fill_price

            order.quantity -= filled_order.quantity

            logger.debug(
                "MATCHED: new order {} vs old order {}", filled_order, matched_order
            )
            logger.debug(
                "SENT: notifications of order execution to agents {} and {} for orders {} and {}",
                filled_order.agent_id,
                matched_order.agent_id,
                filled_order.order_id,
                matched_order.order_id,
            )

            self.owner.send_message(
                matched_order.agent_id, OrderExecutedMsg(matched_order)
            )
            self.owner.send_message(order.agent_id, OrderExecutedMsg(filled_order))

            if self.owner.book_logging == True:
                # append current OB state to book_log2
                self.append_book_log2()

            # Return (only the executed portion of) the matched order.
            return matched_order
}