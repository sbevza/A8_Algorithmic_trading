// src/algo_trading/models/trade_data.cc

#include "trade_data.h"

namespace s21 {

TradeData::TradeData(const double ts, const double c, const double w)
    : timestamp(ts), close(c), weight(w) {}

}  // namespace s21
