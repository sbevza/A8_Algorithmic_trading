// src/algo_trading/models/trade_data.h

#ifndef SRC_MODELS_TRADE_DATA_H_
#define SRC_MODELS_TRADE_DATA_H_

namespace s21 {

struct TradeData {
  double timestamp;
  double close;
  double weight;

  TradeData() : timestamp(0.0), close(0.0), weight(1.0) {}
  TradeData(double ts, double c, double w = 1.0);
};

}  // namespace s21

#endif  // SRC_MODELS_TRADE_DATA_H_
