#ifndef ALGO_TRADING_CONTROLLER_H
#define ALGO_TRADING_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>

// Простая структура для хранения одной записи из CSV (например, свеча или тик)
struct TradeData {
    QDateTime timestamp;
    double open;
    double high;
    double low;
    double close;
    double volume;

    // Конструктор (опционально)
    TradeData() : open(0), high(0), low(0), close(0), volume(0) {}
};

namespace s21 {

class AlgoTradingController : public QObject {
    Q_OBJECT

public:
    explicit AlgoTradingController(QObject *parent = nullptr);
    ~AlgoTradingController();

    // Загружает и парсит CSV
    bool loadTradingDataFromCsv(const QString& content);

    // Получение данных
    const QVector<TradeData>& getTradeData() const;
    int getDataCount() const;

private:
    QVector<TradeData> tradeData_;  // Хранит загруженные рыночные данные
};

}  // namespace s21

#endif // ALGO_TRADING_CONTROLLER_H