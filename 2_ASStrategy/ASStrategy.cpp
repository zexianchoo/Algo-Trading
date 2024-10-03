/*================================================================================                               
*     Source: ../RCM/StrategyStudio/examples/strategies/SimpleMomentumStrategy/SimpleMomentumStrategy.cpp                                                        
*     Last Update: 2013/6/1 13:55:14                                                                            
*     Contents:                                     
*     Distribution:          
*                                                                                                                
*                                                                                                                
*     Copyright (c) RCM-X, 2011 - 2013.                                                  
*     All rights reserved.                                                                                       
*                                                                                                                
*     This software is part of Licensed material, which is the property of RCM-X ("Company"), 
*     and constitutes Confidential Information of the Company.                                                  
*     Unauthorized use, modification, duplication or distribution is strictly prohibited by Federal law.         
*     No title to or ownership of this software is hereby transferred.                                          
*                                                                                                                
*     The software is provided "as is", and in no event shall the Company or any of its affiliates or successors be liable for any 
*     damages, including any lost profits or other incidental or consequential damages relating to the use of this software.       
*     The Company makes no representations or warranties, express or implied, with regards to this software.                        
/*================================================================================*/   

#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "ASStrategy.h"



using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

using namespace std;


ASStrategy::ASStrategy(StrategyID strategyID, const std::string& strategyName, const std::string& groupName):
    Strategy(strategyID, strategyName, groupName)  

{
    Time = 1;
    sigma = 0.2;
    q = 1;
    gamma = 0.4;
    x = 1;
    k = 0.3;
    net_quantity = 1;
    inventory_threshold = 0.75;
    take_quantity =1;
    MAX_POSITION = 1000000;

}

ASStrategy::~ASStrategy()
{

}

void ASStrategy::DefineStrategyCommands()
{
    StrategyCommand command1(1, "Reprice Existing Orders");
    commands().AddCommand(command1);

    StrategyCommand command2(2, "Cancel All Orders");
    commands().AddCommand(command2);
}

void ASStrategy::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate)
{    
}










void ASStrategy::OnTrade(const TradeDataEventMsg& msg)
{

}


void ASStrategy::OnTopQuote(const QuoteEventMsg& msg)
{
	// std::cout << "OnTopQuote(): (" << msg.adapter_time() << "): " << msg.instrument().symbol() << ": " <<
	// 	msg.instrument().top_quote().bid_size() << " @ $"<< msg.instrument().top_quote().bid() <<
	// 	msg.instrument().top_quote().ask_size() << " @ $"<< msg.instrument().top_quote().ask() <<
	// 	std::endl;

}

void ASStrategy::OnQuote(const QuoteEventMsg& msg)
{
	// std::cout << "OnQuote(): (" << msg.adapter_time() << "): " << msg.instrument().symbol() << ": " <<
	// 		msg.instrument().top_quote().bid_size() << " @ $"<< msg.instrument().top_quote().bid() <<
	// 		msg.instrument().top_quote().ask_size() << " @ $"<< msg.instrument().top_quote().ask() <<
	// 		std::endl;
}

void ASStrategy::OnDepth(const MarketDepthEventMsg& msg)
{

    std::cout<<"OnDepth function is called at " << msg.source_time()<< std::endl;
    double bid_price = msg.instrument().top_quote().bid();
    double ask_price = msg.instrument().top_quote().ask();

    double mid_price = (bid_price + ask_price)/2;
    double spread = (bid_price - ask_price)/2;
    cout << msg.source_time() << endl;

    int i = 0.05;

    double r = mid_price - net_quantity * gamma * sigma*sigma * (1 - i * (1/24*60*60));
    spread = spread / 2;

    double gap = abs(mid_price - r);

    double r_a = 0;
    double r_b = 0;
    double delta_a = 0;
    double delta_b = 0;

    if(r > mid_price)
    {

            delta_a = spread + gap;
            delta_b = spread - gap;

    }
    else{

            delta_a = spread + gap;
            delta_b = spread - gap;
    }

    r_a = r + delta_a;
    r_b = r - delta_b;
    int buy =0;
    int sell =0;

    if (mid_price > r_a){
        sell =1;
    }
    if(mid_price < r_b){
        buy =1;
    }


    const Instrument* cur_instrument = &msg.instrument();

    if (buy > 0){
        SendOrder(cur_instrument, buy, bid_price);
    }

    if (sell > 0){
        SendOrder(cur_instrument, sell, ask_price);
    }

    AdjustInventory(cur_instrument);

	std::cout << "-----------------------------------------------------" << std::endl;

}

void ASStrategy::OnBar(const BarEventMsg& msg)
{

}

void ASStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg)
{    
	std::cout << "OnOrderUpdate(): " << msg.update_time() << msg.name() << std::endl;
}

void ASStrategy::OnStrategyCommand(const StrategyCommandEventMsg& msg)
{
    switch (msg.command_id()) {
        case 1:
//            RepriceAll();
            break;
        case 2:
            trade_actions()->SendCancelAll();
            break;
        default:
            logger().LogToClient(LOGLEVEL_DEBUG, "Unknown strategy command received");
            break;
    }
}

void ASStrategy::SendOrder(const Instrument* instrument, int quantity, double price) 
{
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

void ASStrategy::AdjustInventory(const Instrument* instrument) 
{
    if(((double)net_quantity / MAX_POSITION) > inventory_threshold) 
    {
        SendOrder(instrument, (-1 * take_quantity), instrument->top_quote().ask());
    } else if (((double)net_quantity / MAX_POSITION) < (-1 * inventory_threshold)) 
    {
        SendOrder(instrument, take_quantity, instrument->top_quote().bid());
    }
}




void ASStrategy::OnParamChanged(StrategyParam& param)
{    

}

void ASStrategy::OnResetStrategyState()
{

}

void ASStrategy::DefineStrategyParams()
{

}
