// trade_data.h
#ifndef SRC_MODELS_TRADE_DATA_H_
#define SRC_MODELS_TRADE_DATA_H_

#include <QDateTime>

namespace s21 {

struct TradeData {
    QDateTime timestamp;
    double close;
    double weight;

    TradeData() : close(0.0), weight(1.0) {}
    TradeData(const QDateTime& ts, double c, double w = 1.0);
};

}  // namespace s21

#endif  // SRC_MODELS_TRADE_DATA_H_
