// Refer to the header files for the explanation of the functions in detail
// In VSCode, hovering your mouse above the function renders the explanation of from the .h file as a pop up
#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "SigmoidStrategy.h"

// Constructor to initialize member variables of the class to their initial values.
SigmoidStrategy::SigmoidStrategy(StrategyID strategyID,
                    const std::string& strategyName,
                    const std::string& groupName):
    Strategy(strategyID, strategyName, groupName) {

    // Purpose of each variable explained in the header file.
    target_spread = 0.01;
    position_target = 1000;
    max_pos = 5000.0;
    min_pos = -5000.0;
    interval_time = 1;

    buy_price = 0; // price on top of the bid side whenever we buy
    sell_price = 0; 

    trade_size_each_time = 10; // the number of mid-prices used to calculate the big moving average.
    total_stock_position = 0.0; // the number of shares of the instrument currently held in the strategy's portfolio.
    total_revenue = 0.0; // the amount of cash (positive or negative) earned from buying and selling the instrument.
    total_trade_count = 0;

    sigmoid_center = 0.5;
    sigmoid_scale = 10.0;

}

// Destructor for class
SigmoidStrategy::~SigmoidStrategy() {
}

void SigmoidStrategy::DefineStrategyParams() {
}

// By default, SS will register to trades/quotes/depth data for the instruments you have requested via command_line.
void SigmoidStrategy::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate) {
}

void SigmoidStrategy::OnDepth(const MarketDepthEventMsg& msg) {

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
        

        
    // std::cout << "Top quote was updated. Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;
    

    // Calculate the midpoint price
    double midpoint_price = (best_bid_price + best_ask_price) / 2.0;

    // Calculate the bid and ask prices for market making
    double market_bid_price = midpoint_price - (target_spread / 2.0);
    double market_ask_price = midpoint_price + (target_spread / 2.0);

    // Sigmoid function to adjust the trade size based on the current position
    double sigmoid_input = (double)(total_stock_position - position_target) / position_target;
    double sigmoid_exp = sigmoid_scale * (sigmoid_input - sigmoid_center);
    if (sigmoid_exp > 0) {
        sigmoid_exp = -sigmoid_exp;
    }
    double sigmoid_adjustment = 1 / (1 + exp(sigmoid_exp));
    double adjusted_trade_size = static_cast<double>(trade_size_each_time * sigmoid_adjustment);

    // Ensure the trade size is within the position limits
    adjusted_trade_size = std::min(adjusted_trade_size, max_pos - total_stock_position);
    adjusted_trade_size = std::max(adjusted_trade_size, min_pos - total_stock_position);
    
    std::cout << "Sigmoid Input: " << sigmoid_input << " Sigmoid Adjustment: " << sigmoid_adjustment <<
    " Adjusted trade_size: " << adjusted_trade_size << std::endl;

    // Market Making: Place bid and ask orders based on the calculated prices
    if (total_stock_position < max_pos) {
        std::cout << "Before Buy order " << " Total Stock Position: " << total_stock_position << std::endl;
        // Place a bid order at the calculated market bid price
        SendOrder(instrument, adjusted_trade_size, market_ask_price, market_bid_price); //Bid
        total_stock_position += adjusted_trade_size;  // Update total position
        total_revenue -= market_bid_price * adjusted_trade_size;  // Cash outflow
        total_trade_count += 1;  // Increment trade count
    }

    if (total_stock_position > min_pos) {
        std::cout << "Before Sell order " << " Total Stock Position: " << total_stock_position << std::endl;
        // Place an ask order at the calculated market ask price
        SendOrder(instrument, -adjusted_trade_size, market_ask_price, market_bid_price); //Ask
        total_stock_position -= adjusted_trade_size;  // Update total position
        total_revenue += market_ask_price * adjusted_trade_size;  // Cash inflow
        total_trade_count += 1;  // Increment trade count
    }

    }

    // std::cout<<"OnDepth function is called at " << msg.source_time() << ". Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;
    // std::cout<< buy_position << " " << sell_position << std::endl;
    std::cout <<"OnDepth called" << std::endl;
}

void SigmoidStrategy::OnTopQuote(const QuoteEventMsg& msg) {
    // Code to test the OnTopQuote function. Uncomment the code below to test the function.
    // const Quote& quote = msg.quote();

    // double best_bid_price = quote.bid();
    // double best_ask_price = quote.ask();

    // std::cout<<"OnTopQuote function is called at " << msg.source_time() << ". Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;
}


void SigmoidStrategy::OnTrade(const TradeDataEventMsg& msg) {
    // Code to test the OnTrade function. Uncomment the code below to test the function.
    // const Trade& trade = msg.trade();
    // double trade_price = trade.price();
    // int trade_size = trade.size();

    // std::cout<<"OnTrade function is called. " << trade_size << " @ " << trade_price << std::endl;
}

void SigmoidStrategy::OnScheduledEvent(const ScheduledEventMsg& msg) {
}

void SigmoidStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg) {

    // // Examples of other information that can be accessed through the msg object, but they are not used in this implementation.
    // std::cout << "name = " << msg.name() << std::endl;
    // std::cout << "order id = " << msg.order_id() << std::endl;
    // std::cout << "fill occurred = " << msg.fill_occurred() << std::endl;
    // std::cout << "update type = " << msg.update_type() << std::endl;

    // Update time of the order update event is printed to the console using std::cout.
    std::cout << "time " << msg.update_time() << std::endl;

}

void SigmoidStrategy::OnBar(const BarEventMsg& msg) {
    // Code to test the OnBar function. Uncomment the code below to test the function.
    // const Instrument& instrument = msg.instrument();

    // double best_bid_price = instrument.nbbo().bid();
    // double best_ask_price = instrument.nbbo().ask();

    // std::cout<<"OnBar function is called. Best Bid: " << best_bid_price << " Best Ask: " << best_ask_price << std::endl;
}

void SigmoidStrategy::AdjustPortfolio() {
}

void SigmoidStrategy::SendOrder(const Instrument* instrument, int trade_size_each_time, double ask, double bid) {

    double price;

    if (trade_size_each_time > 0) { // buy
        price = bid;
    } else { // sell
        price = ask;
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

void SigmoidStrategy::OnResetStrategyState() {
}

void SigmoidStrategy::OnParamChanged(StrategyParam& param) {
}