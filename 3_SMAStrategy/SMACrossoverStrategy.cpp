#ifdef _WIN32
#include "stdafx.h"
#endif

#include "SMACrossoverStrategy.h"
#include <numeric>

using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

SMACrossoverStrategy::SMACrossoverStrategy(StrategyID strategyID,
                                           const std::string& strategyName,
                                           const std::string& groupName)
    : Strategy(strategyID, strategyName, groupName) 
    {
    shortPeriod_ = 20;
    longPeriod_ = 50;
    position = 0.0;
}

SMACrossoverStrategy::~SMACrossoverStrategy() {}

void SMACrossoverStrategy::DefineStrategyParams() {
    params().CreateParam(CreateStrategyParamArgs("shortPeriod", STRATEGY_PARAM_TYPE_STARTUP, VALUE_TYPE_INT, shortPeriod_));
    params().CreateParam(CreateStrategyParamArgs("longPeriod", STRATEGY_PARAM_TYPE_STARTUP, VALUE_TYPE_INT, longPeriod_));
}

void SMACrossoverStrategy::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate) {

    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 10);
    }

    // eventRegister->RegisterForDepthEvent(this);
}

void SMACrossoverStrategy::OnDepth(const MarketDepthEventMsg& msg) {
    const Instrument& instrument = msg.instrument();
    prices.push_back(instrument.top_quote().mid_price());

    if (prices.size() >= std::max(shortPeriod_, longPeriod_)) {
        shortSMA.clear();
        longSMA.clear();

        for (int i = static_cast<int>(prices.size()) - 1; i >= 0; --i) {
            shortSMA.push_back(std::accumulate(prices.begin() + std::max(0, i - shortPeriod_ + 1), prices.begin() + i + 1, 0.0) / shortPeriod_);
            longSMA.push_back(std::accumulate(prices.begin() + std::max(0, i - longPeriod_ + 1), prices.begin() + i + 1, 0.0) / longPeriod_);
        }

        std::reverse(shortSMA.begin(), shortSMA.end());
        std::reverse(longSMA.begin(), longSMA.end());

        AdjustPortfolio(&instrument);
    }
}

void SMACrossoverStrategy::OnBar(const BarEventMsg& msg) {}

void SMACrossoverStrategy::AdjustPortfolio(const Instrument* instrument) {
    int currentIndex = static_cast<int>(prices.size()) - 1;

    if (currentIndex >= static_cast<int>(std::max(shortPeriod_, longPeriod_)) - 1) {
        double currentShortSMA = shortSMA[currentIndex];
        double currentLongSMA = longSMA[currentIndex];
        double prevShortSMA = shortSMA[currentIndex - 1];
        double prevLongSMA = longSMA[currentIndex - 1];

        if (currentShortSMA > currentLongSMA && prevShortSMA <= prevLongSMA) {
            SendOrder(instrument, 1000); // Buy signal
        } else if (currentShortSMA < currentLongSMA && prevShortSMA >= prevLongSMA) {
            SendOrder(instrument, -1000); // Sell signal
        }
    }
}

void SMACrossoverStrategy::SendOrder(const Instrument* instrument, int trade_size) {
    double price = instrument->top_quote().mid_price();
    OrderSide side = (trade_size > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL;

    OrderParams params(
        *instrument,
        abs(trade_size),
        price,
        MARKET_CENTER_ID_IEX,
        side,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        std::cout << "Sending new trade order successful!" << std::endl;
        std::cout << "Position: " << position << std::endl;
        position += trade_size;
    } else {
        std::cout << "Error sending new trade order..." << tra << std::endl;
    }
}

void SMACrossoverStrategy::OnResetStrategyState() {
}

void SMACrossoverStrategy::OnParamChanged(StrategyParam& param) {
}


