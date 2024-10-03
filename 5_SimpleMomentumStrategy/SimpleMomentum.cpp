/*================================================================================
*     Source: ../RCM/StrategyStudio/examples/strategies/SimpleMomentumStrategy/SimpleMomentumStrategy.cpp
*     Last Update: 2021/04/15 13:55:14
*     Contents:
*     Distribution:
*
*
*     Copyright (c) RCM-X, 2011 - 2021.
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

#include "SimpleMomentum.h"

#include "FillInfo.h"
#include "AllEventMsg.h"
#include "ExecutionTypes.h"
#include <Utilities/Cast.h>
#include <Utilities/utils.h>

#include <math.h>
#include <iostream>
#include <cassert>

using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

using namespace std;

SimpleMomentum::SimpleMomentum(StrategyID strategyID, const std::string& strategyName, const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    momentum_map_(),
    aggressiveness_(0),
    position_size_(100),
    debug_(false),
    short_window_size_(10),
    long_window_size_(30)
{
    //this->set_enabled_pre_open_data_flag(true);
    //this->set_enabled_pre_open_trade_flag(true);
    //this->set_enabled_post_close_data_flag(true);
    //this->set_enabled_post_close_trade_flag(true);
}

SimpleMomentum::~SimpleMomentum()
{
}

void SimpleMomentum::OnResetStrategyState()
{
    momentum_map_.clear();
}

void SimpleMomentum::DefineStrategyParams()
{
    params().CreateParam(CreateStrategyParamArgs("aggressiveness", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_DOUBLE, aggressiveness_));
    params().CreateParam(CreateStrategyParamArgs("position_size", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_INT, position_size_));
    params().CreateParam(CreateStrategyParamArgs("short_window_size", STRATEGY_PARAM_TYPE_STARTUP, VALUE_TYPE_INT, short_window_size_));
    params().CreateParam(CreateStrategyParamArgs("long_window_size", STRATEGY_PARAM_TYPE_STARTUP, VALUE_TYPE_INT, long_window_size_));
    params().CreateParam(CreateStrategyParamArgs("debug", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_BOOL, debug_));
}

void SimpleMomentum::DefineStrategyCommands()
{
    commands().AddCommand(StrategyCommand(1, "Reprice Existing Orders"));
    commands().AddCommand(StrategyCommand(2, "Cancel All Orders"));
}

void SimpleMomentum::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate)
{    
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 10);
    }
}

void SimpleMomentum::OnBar(const BarEventMsg& msg)
{ 
    if (debug_) {
        ostringstream str;
        str << msg.instrument().symbol() << ": " << msg.bar();
        logger().LogToClient(LOGLEVEL_DEBUG, str.str().c_str());
    }

    //check if we're already tracking the momentum object for this instrument, if not create a new one
    MomentumMapIterator momentum_iter = momentum_map_.find(&msg.instrument());
    if (momentum_iter == momentum_map_.end()) {
        momentum_iter = momentum_map_.insert(make_pair(&msg.instrument(), Momentum(short_window_size_, long_window_size_))).first;
    }

    DesiredPositionSide side = momentum_iter->second.Update(msg.bar().close());

    if (momentum_iter->second.FullyInitialized()) {
        AdjustPortfolio(&msg.instrument(), position_size_ * side);
    }
}

void SimpleMomentum::OnOrderUpdate(const OrderUpdateEventMsg& msg)  
{
}

void SimpleMomentum::AdjustPortfolio(const Instrument* instrument, int desired_position)
{
    int trade_size = desired_position - portfolio().position(instrument);

    if (trade_size != 0) {
        // if we're not working an order for the instrument already, place a new order
        if (orders().num_working_orders(instrument) == 0) {
            SendOrder(instrument, trade_size);
        } else {  
            // otherwise find the order and cancel it if we're now trying to trade in the other direction
            const Order* order = *orders().working_orders_begin(instrument);
            if (order && ((IsBuySide(order->order_side()) && trade_size < 0) || 
                         ((IsSellSide(order->order_side()) && trade_size > 0)))) {
                trade_actions()->SendCancelOrder(order->order_id());
                // we're avoiding sending out a new order for the other side immediately to simplify the logic to the case where we're only tracking one order per instrument at any given time
            }
        }
    }
}

void SimpleMomentum::SendOrder(const Instrument* instrument, int trade_size)
{
    if (!instrument->top_quote().ask_side().IsValid() || !instrument->top_quote().ask_side().IsValid()) {
        std::stringstream ss;
        ss << "Skipping trade due to lack of two sided quote"; 
        logger().LogToClient(LOGLEVEL_DEBUG, ss.str());
        return;
     }

    double price = trade_size > 0 ? instrument->top_quote().bid() + aggressiveness_ : instrument->top_quote().ask() - aggressiveness_;

    OrderParams params(*instrument,
                       abs(trade_size),
                       price,
                       (instrument->type() == INSTRUMENT_TYPE_EQUITY) ? MARKET_CENTER_ID_NASDAQ : ((instrument->type() == INSTRUMENT_TYPE_OPTION) ? MARKET_CENTER_ID_CBOE_OPTIONS : MARKET_CENTER_ID_CME_GLOBEX),
                       (trade_size > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
                       ORDER_TIF_DAY,
                       ORDER_TYPE_LIMIT);

    trade_actions()->SendNewOrder(params);
}

void SimpleMomentum::RepriceAll()
{
    for (IOrderTracker::WorkingOrdersConstIter ordit = orders().working_orders_begin(); ordit != orders().working_orders_end(); ++ordit) {
        Reprice(*ordit);
    }
}

void SimpleMomentum::Reprice(Order* order)
{
    OrderParams params = order->params();
    params.price = (order->order_side() == ORDER_SIDE_BUY) ? order->instrument()->top_quote().bid() + aggressiveness_ : order->instrument()->top_quote().ask() - aggressiveness_;
    trade_actions()->SendCancelReplaceOrder(order->order_id(), params);
}

void SimpleMomentum::OnStrategyCommand(const StrategyCommandEventMsg& msg)
{
    switch (msg.command_id()) {
        case 1:
            RepriceAll();
            break;
        case 2:
            trade_actions()->SendCancelAll();
            break;
        default:
            logger().LogToClient(LOGLEVEL_DEBUG, "Unknown strategy command received");
            break;
    }
}

void SimpleMomentum::OnParamChanged(StrategyParam& param)
{    
    if (param.param_name() == "aggressiveness") {
        if (!param.Get(&aggressiveness_))
            throw StrategyStudioException("Could not get aggressiveness");
    } else if (param.param_name() == "position_size") {
        if (!param.Get(&position_size_))
            throw StrategyStudioException("Could not get position_size");
    } else if (param.param_name() == "short_window_size") {
        if (!param.Get(&short_window_size_))
            throw StrategyStudioException("Could not short_window_size");
    } else if (param.param_name() == "long_window_size") {
        if (!param.Get(&long_window_size_))
            throw StrategyStudioException("Could not get long_window_size");
    } else if (param.param_name() == "debug") {
        if (!param.Get(&debug_))
            throw StrategyStudioException("Could not get debug");
    } 
}