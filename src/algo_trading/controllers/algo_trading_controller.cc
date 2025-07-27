// algo_trading_controller.cc
#include "algo_trading_controller.h"
#include <QDebug>

namespace s21 {

AlgoTradingController::AlgoTradingController(QObject *parent)
    : QObject(parent) // parser_ инициализируется автоматически
{
    // Ничего не нужно делать — parser_ уже создан
}


bool AlgoTradingController::loadTradingDataFromCsv(const QString &content) {
    tradeData_.clear();
    auto parsed_data = parser_.Parse(content); // ✅ Используем .

    if (parsed_data.isEmpty()) {
        qWarning() << "Failed to parse CSV:" << parser_.GetError(); // ✅ .
        return false;
    }

    tradeData_ = parsed_data;
    qDebug() << "Successfully loaded" << tradeData_.size() << "records";
    return true;
}

const QVector<TradeData>& AlgoTradingController::getTradeData() const {
    return tradeData_;
}

int AlgoTradingController::getDataCount() const {
    return tradeData_.size();
}

QString AlgoTradingController::getLastError() const {
    return parser_.GetError();
}
}  // namespace s21
