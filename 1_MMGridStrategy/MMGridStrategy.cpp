// Refer to the header files for the explanation of the functions in detail
// In VSCode, hovering your mouse above the function renders the explanation of from the .h file as a pop up
#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "MMGridStrategy.h"

// Constructor to initialize member variables of the class to their initial values.
MMGridStrategy::MMGridStrategy(StrategyID strategyID,
                    const std::string& strategyName,
                    const std::string& groupName):
    Strategy(strategyID, strategyName, groupName) {

    // Purpose of each variable explained in the header file.
    trade_size_each_time = 10;
	buy_price = 0;
    sell_price = 0;
	total_revenue = 0.0;
    total_trade_count = 0;
    buy_position = 0;
    sell_position = 0;

}

// Destructor for class
MMGridStrategy::~MMGridStrategy() {
}

void MMGridStrategy::DefineStrategyParams() {
}

// By default, SS will register to trades/quotes/depth data for the instruments you have requested via command_line.
void MMGridStrategy::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate) {
}

void MMGridStrategy::OnDepth(const MarketDepthEventMsg& msg) {

    const Instrument* instrument = &msg.instrument();
    double best_bid_price = msg.instrument().top_quote().bid();
    double best_ask_price = msg.instrument().top_quote().ask();
    // double best_bid_price = msg.instrument().aggregate_order_book().BidPriceLevelAtLevel(1)->price();
    // double best_ask_price = msg.instrument().aggregate_order_book().AskPriceLevelAtLevel(1)->price();
    std::cout << "Top quote was updated. Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;

    if (msg.updated_top_quote()) {
        const Quote quote = instrument->top_quote();
        std::string symbol = instrument->symbol();
        double min_tick_size = instrument->min_tick_size();

        double best_bid_price = quote.bid();
        double best_ask_price = quote.ask();
        

        
    std::cout << "Top quote was updated. Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;
    

    if (buy_position == 0) {

        SendOrder(instrument, trade_size_each_time);
        buy_position += trade_size_each_time;
        buy_price = best_bid_price;

    } 
    // else if (best_bid_price > buy_price + 0.05) { 
    //     buy_position--;
    //     total_revenue += (best_bid_price - buy_price) * trade_size_each_time;
    //     total_trade_count++;
    //     buy_price = 0;

    //     SendOrder(instrument, -trade_size_each_time);
    // }

    if (sell_position == 0) {

        SendOrder(instrument, -trade_size_each_time);
        sell_position -= trade_size_each_time;
        sell_price = best_ask_price;

    }

    }

    // std::cout<<"OnDepth function is called at " << msg.source_time() << ". Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;
    // std::cout<< buy_position << " " << sell_position << std::endl;
    std::cout <<"OnDepth called" << std::endl;
}

void MMGridStrategy::OnTopQuote(const QuoteEventMsg& msg) {
    // Code to test the OnTopQuote function. Uncomment the code below to test the function.
    // const Quote& quote = msg.quote();

    // double best_bid_price = quote.bid();
    // double best_ask_price = quote.ask();

    // std::cout<<"OnTopQuote function is called at " << msg.source_time() << ". Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;
}


void MMGridStrategy::OnTrade(const TradeDataEventMsg& msg) {
    // Code to test the OnTrade function. Uncomment the code below to test the function.
    // const Trade& trade = msg.trade();
    // double trade_price = trade.price();
    // int trade_size = trade.size();

    // std::cout<<"OnTrade function is called. " << trade_size << " @ " << trade_price << std::endl;
}

void MMGridStrategy::OnScheduledEvent(const ScheduledEventMsg& msg) {
}

void MMGridStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg) {

    // // Examples of other information that can be accessed through the msg object, but they are not used in this implementation.
    // std::cout << "name = " << msg.name() << std::endl;
    // std::cout << "order id = " << msg.order_id() << std::endl;
    // std::cout << "fill occurred = " << msg.fill_occurred() << std::endl;
    // std::cout << "update type = " << msg.update_type() << std::endl;

    // Update time of the order update event is printed to the console using std::cout.
    std::cout << "time " << msg.update_time() << std::endl;

}

void MMGridStrategy::OnBar(const BarEventMsg& msg) {
    // Code to test the OnBar function. Uncomment the code below to test the function.
    // const Instrument& instrument = msg.instrument();

    // double best_bid_price = instrument.nbbo().bid();
    // double best_ask_price = instrument.nbbo().ask();

    // std::cout<<"OnBar function is called. Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;
}

void MMGridStrategy::AdjustPortfolio() {
}

void MMGridStrategy::SendOrder(const Instrument* instrument, int trade_size_each_time) {

    double price;

    if (trade_size_each_time > 0) { // buy
        price = instrument->top_quote().ask();
    } else { // sell
        price = instrument->top_quote().bid();
    }

    // Create an order object with the specified parameters
    OrderParams params(
                    *instrument,     // Instrument to trade
                    abs(trade_size_each_time), // Absolute value of trade size
                    price,           // Price at which to trade
                    MARKET_CENTER_ID_IEX, // Market center ID
                    (trade_size_each_time > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL, // Order side (buy or sell)
                    ORDER_TIF_DAY,   // Time in force (how long the order is valid for)
                    ORDER_TYPE_LIMIT // Order type (limit or market)
                    );

    // During the first run of your backtest, you would've seen a lot of lines being printed out to the console.
    // Below are those lines. I've commented them out to reduce clutter and speed up the backtest (printing many lines slows down the backtest)
    std::string action;
    if (trade_size_each_time > 0) {
            action = "buy ";
    } else {
            action = "sell ";
    }

    // Print a message indicating that a new order is being sent
    std::cout << "SendTradeOrder(): about to send new order for size "
            << trade_size_each_time
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
    } else {
            std::cout << "Error sending new trade order..." << tra << std::endl;
    }

}

void MMGridStrategy::OnResetStrategyState() {
}

void MMGridStrategy::OnParamChanged(StrategyParam& param) {
}