#include "../util/timestamping.h"
#include "ExchangeAgent.h"
#include "../message/orders.h"

ExchangeAgent::ExchangeAgent(
    int id, 
    Timestamp mkt_open,
    Timestamp mkt_close,
    std::vector<std::string> symbols,
    Logger& logger,
    std::optional<std::string> name,
    std::optional<std::string> type,
    bool book_logging,
    int book_log_depth,
    int pipeline_delay,
    int computational_delay,
    int stream_history,
    bool log_orders,
    int random_state,
    bool use_metric_tracker
    ) : FinancialAgent(id, name, type, random_state, logger), symbols(symbols),
      // Store this exchange's open and close times.
      mkt_open(mkt_open), mkt_close(mkt_close),  

      // Right now, only the exchange agent has a parallel processing pipeline delay.  This is an additional
      // delay added only to order activity (placing orders, etc) and not simple inquiries (market operating
      // hours, etc).
      pipeline_delay(pipeline_delay),

      // Computation delay is applied on every wakeup call or message received.
      computational_delay(computational_delay),

      // The exchange maintains an order stream of all orders leading to the last L trades
      // to support certain agents from the auction literature (GD, HBL, etc).
      stream_history(stream_history), 

      book_logging(book_logging), book_log_depth(book_log_depth), log_orders(log_orders)

      {
        // Do not request repeated wakeup calls.
        reschedule = false;

        // Create an order book for each symbol.
        


    # Create an order book for each symbol.
    self.order_books: Dict[str, OrderBook] = {
        symbol: OrderBook(self, symbol) for symbol in symbols
    }

    if use_metric_tracker:
        # Create a metric tracker for each symbol.
        self.metric_trackers: Dict[str, ExchangeAgent.MetricTracker] = {
            symbol: self.MetricTracker() for symbol in symbols
        }

    # The subscription dict is a dictionary with the key = agent ID,
    # value = dict (key = symbol, value = list [levels (no of levels to recieve updates for),
    # frequency (min number of ns between messages), last agent update timestamp]
    # e.g. {101 : {'AAPL' : [1, 10, NanosecondTime(10:00:00)}}
    self.data_subscriptions: DefaultDict[
        str, List[ExchangeAgent.BaseDataSubscription]
    ] = defaultdict(list)

    # Store a list of agents who have requested market close price information.
    # (this is most likely all agents)
    self.market_close_price_subscriptions: List[int] = []
    }