// algo_trading_controller.h
#ifndef SRC_CONTROLLERS_ALGO_TRADING_CONTROLLER_H_
#define SRC_CONTROLLERS_ALGO_TRADING_CONTROLLER_H_

#include <QObject>
#include <QVector>
#include "../models/csv_parser.h"

namespace s21 {

class AlgoTradingController : public QObject {
    Q_OBJECT

public:
    explicit AlgoTradingController(QObject *parent = nullptr);

    bool loadTradingDataFromCsv(const QString& content);
    const QVector<TradeData>& getTradeData() const;
    int getDataCount() const;
    QString getLastError() const;

private:
    QVector<TradeData> tradeData_;
    CsvParser parser_;
};

}  // namespace s21

#endif  // SRC_CONTROLLERS_ALGO_TRADING_CONTROLLER_H_
