// trade_data.cc
#include "trade_data.h"

namespace s21 {

TradeData::TradeData(const QDateTime& ts, double c, double w)
    : timestamp(ts), close(c), weight(w) {}

}  // namespace s21
