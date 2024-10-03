// Refer to the header files for the explanation of the functions in detail
// In VSCode, hovering your mouse above the function renders the explanation of from the .h file as a pop up
#ifdef _WIN32
#include "stdafx.h"
#endif

#include "OfiStrategy.h"

// Constructor to initialize member variables of the class to their initial values.
OfiStrategy::OfiStrategy(StrategyID strategyID,
                    const std::string& strategyName,
                    const std::string& groupName):
    Strategy(strategyID, strategyName, groupName) {

    // Purpose of each variable explained in the header file.
	total_revenue = 0.0;
    total_trade_count = 0;
    buy_position = 0;
    sell_position = 0;
    max_buy_position = 10000;
    max_sell_position = -10000;
    imbalance_threshold = 1000;

}

// Destructor for class
OfiStrategy::~OfiStrategy() {
}

void OfiStrategy::DefineStrategyParams() {
}

// By default, SS will register to trades/quotes/depth data for the instruments you have requested via command_line.
void OfiStrategy::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate) {
}

void OfiStrategy::OnTrade(const TradeDataEventMsg& msg) {
    // std::cout << "OnTrade event occurred: " << msg.name() << std::endl;
}

void OfiStrategy::OnImbalance(const ImbalanceEventMsg & msg) {

    // This function is not called during the backtest. Ignore code in this function.

    std::cout << "Imbalance event occurred: " << msg.name() << std::endl;

    const Instrument* instrument = &msg.instrument();

    // Number of shares that are out of balance
    // Normalized so all feeds have a signed value, positive for more buy orders than sell orders
    int imbalance_size = msg.imbalance_size();

    // Closest indicative price where imbalance would be zero
    double clearing_price = msg.clearing_price();

    // Reference price corresponding to the imbalance quantities
    double reference_price = msg.reference_price();

    if (buy_position < max_buy_position) {
        int quantity = std::max(max_buy_position - buy_position, 0);
        if (imbalance_size > 0) { // more buy orders than sell orders
            SendOrder(instrument, quantity, clearing_price); // buy at clearing price
        } else if (imbalance_size < 0) { // more sell orders than buy orders
            SendOrder(instrument, quantity, reference_price); // buy at reference price
        }
    }

    if (sell_position < max_sell_position) {
        int quantity = std::max(max_sell_position - sell_position, 0);
        if (imbalance_size > 0) { // more buy orders than sell orders
            SendOrder(instrument, -quantity, reference_price); // sell at reference price
        } else if (imbalance_size < 0) { // more sell orders than buy orders
            SendOrder(instrument, -quantity, clearing_price); // sell at clearing price
        }
    }

}

void OfiStrategy::OnDepth(const MarketDepthEventMsg& msg) {
    // std::cout <<"OnDepth called" << std::endl;
    const Instrument* instrument = &msg.instrument();
    const Quote& top_quote = instrument->top_quote();

    double best_bid_price = top_quote.bid();
    double best_ask_price = top_quote.ask();
    int best_bid_size = top_quote.bid_size();
    int best_ask_size = top_quote.ask_size();

    // std::cout << "Top quote was updated. Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;

    int imbalance = best_bid_size - best_ask_size;

    if (abs(imbalance) > imbalance_threshold) {
        if (imbalance > 0 && sell_position > max_sell_position) { 
            // more buy orders than sell orders
            // create a sell order at the best ask price
            SendOrder(instrument, -imbalance, best_ask_price);
        } else if (imbalance < 0 && buy_position < max_buy_position) {
            // more sell orders than buy orders
            // create a buy order at the best bid price
            SendOrder(instrument, -imbalance, best_bid_price);
        }
    }
}


void OfiStrategy::OnScheduledEvent(const ScheduledEventMsg& msg) {
}

void OfiStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg) {

    // When a order is completed remove the size of the order from the position

    if (msg.completes_order()) {
        const Order& order = msg.order();
        int size_completed = order.size_completed();
        std::cout << "Order completed: " << size_completed << std::endl;
        if (size_completed > 0) {
            buy_position -= size_completed;
        } else {
            sell_position -= size_completed;
        }
    }

}

void OfiStrategy::OnBar(const BarEventMsg& msg) {
}

void OfiStrategy::AdjustPortfolio() {
}

void OfiStrategy::SendOrder(const Instrument* instrument, int quantity, double price) {

    // Create an order object with the specified parameters
    OrderParams params(
                    *instrument,     // Instrument to trade
                    abs(quantity), // Absolute value of trade size
                    price,           // Price at which to trade
                    MARKET_CENTER_ID_IEX, // Market center ID
                    (quantity > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL, // Order side (buy or sell)
                    ORDER_TIF_DAY,   // Time in force (how long the order is valid for)
                    ORDER_TYPE_LIMIT // Order type (limit or market)
                    );

    // During the first run of your backtest, you would've seen a lot of lines being printed out to the console.
    // Below are those lines. I've commented them out to reduce clutter and speed up the backtest (printing many lines slows down the backtest)
    std::string action;
    if (quantity > 0) {
            action = "buy ";
    } else {
            action = "sell ";
    }

    // Print a message indicating that a new order is being sent
    std::cout << "SendTradeOrder(): about to send new order for size "
            << quantity
            << " at $"
            << price
            << " to "
            << action
            << instrument->symbol()
            << std::endl;
    
    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    // Check if the order was sent successfully and print a message indicating the result
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        std::cout << "Sending new trade order successful!" << std::endl;
        if (quantity > 0) {
            buy_position += quantity;
        } else {
            sell_position += quantity;
        }
    } else {
        std::cout << "Error sending new trade order..." << tra << std::endl;
    }

}

void OfiStrategy::OnResetStrategyState() {
}

void OfiStrategy::OnParamChanged(StrategyParam& param) {
}

