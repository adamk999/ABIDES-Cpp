#include <tuple>
#include <vector>
#include <optional>
#include "../message/orders.h"

typedef std::tuple<LimitOrder, std::optional<std::unordered_map<std::string, int>>> OrderTuple;
typedef std::vector<OrderTuple> OrderList;

struct PriceLevel {
    /*
    A class that represents a single price level containing multiple orders for one
    side of an order book. The option to have hidden orders is supported. This class
    abstracts the complextity of handling both visible and hidden orders away from
    the parent order book.

    Visible orders are consumed first, followed by any hidden orders.

    Attributes:
        visible_orders: A list of visible orders, where the order with index=0 is first
            in the queue and will be exexcuted first.
        hidden_orders: A list of hidden orders, where the order with index=0 is first
            in the queue and will be exexcuted first.
        price: The price this PriceLevel represents.
        side: The side of the market this PriceLevel represents.
    */
    OrderList visible_orders;
    OrderList hidden_orders;
    int price;
    Side side;

    PriceLevel(OrderList orders);
    /*
    Arguments:
        orders: A list of orders, containing both visible and hidden orders that
            will be correctly allocated on initialisation. At least one order must
            be given.
    */
    
    void addOrder(const LimitOrder& order, std::optional<std::unordered_map<std::string, int>> metadata = std::nullopt);
    /*
    Adds an order to the correct queue in the price level.

    Orders are added to the back of their respective queue.

    Arguments:
        order: The `LimitOrder` to add, can be visible or hidden.
        metadata: Optional dict of metadata values to associate with the order.
    */


    bool updateOrderQuantity(int order_id, int new_quantity);
    /*
    Updates the quantity of an order.

    The new_quantity must be greater than 0. To remove an order from the price
    level use the `remove_order` method instead.

    If the new quantity is less than or equal to the current quantity the order's
    position in its respective queue will be maintained.

    If the new quantity is more than the current quantity the order will be moved
    to the back of its respective queue.

    Arguments:
        order_id: The ID of the order to update.
        quantity: The new quantity to update with.

    Returns:
        True if the update was sucessful, False if a matching order with the
        given ID could not be found or if the new quantity given is 0.
    */


    std::optional<OrderTuple> removeOrder(int order_id);
    /*
    Attempts to remove an order from the price level.

    Arguments:
        order_id: The ID of the order to remove.

    Returns:
        The order object if the order was found and removed, else None.
    */


    OrderTuple peek();
    /*
    Returns the highest priority order in the price level. Visible orders are returned first,
    followed by hidden orders if no visible order exist.

    Raises a ValueError exception if the price level has no orders.
    */

    OrderTuple pop();
    /*
    Removes the highest priority order in the price level and returns it. Visible
    orders are returned first, followed by hidden orders if no visible order exist.

    Raises a ValueError exception if the price level has no orders.
    */
    
    bool orderIsMatch(LimitOrder order);
    /*
    Checks if an order on the opposite side of the book is a match with this price
    level.

    The given order must be a `LimitOrder`.

    Arguments:
        order: The order to compare.

    Returns:
        True if the order is a match.
    */

    bool orderHasBetterPrice(LimitOrder order);
    /*
    Checks if an order on this side of the book has a better price than this price
    level.

    Arguments:
        order: The order to compare.

    Returns:
        True if the given order has a better price.
    */
    
    bool orderHasWorsePrice(LimitOrder order);
    /*
    Checks if an order on this side of the book has a worse price than this price
    level.

    Arguments:
        order: The order to compare.

    Returns:
        True if the given order has a worse price.
    */

    bool orderHasEqualPrice(LimitOrder order);
    /*
    Checks if an order on this side of the book has an equal price to this price
    level.

    Arguments:
        order: The order to compare.

    Returns:
        True if the given order has an equal price.
    */

    
    int totalQuantity();
    /*
    Returns the total visible order quantity of this price level.
    */    


    bool isEmpty();
    /*
    Returns True if this price level has no orders.
    */
      

    // def __eq__(self, other: object) -> bool:
    //     if not isinstance(other, PriceLevel):
    //         raise NotImplementedError

    //     return (
    //         self.visible_orders == other.visible_orders
    //         and self.hidden_orders == other.hidden_orders
    //     )

};
        