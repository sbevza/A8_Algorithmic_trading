#include "algo_trading_controller.h"
#include <QDebug>
#include <QStringList>
#include <QDate>

namespace s21 {

AlgoTradingController::AlgoTradingController(QObject *parent)
    : QObject(parent) {}

AlgoTradingController::~AlgoTradingController() = default;

bool AlgoTradingController::loadTradingDataFromCsv(const QString &content) {
    tradeData_.clear();

    if (content.isEmpty()) {
        qWarning() << "CSV content is empty";
        return false;
    }

    QStringList lines = content.trimmed().split('\n', Qt::SkipEmptyParts);

    int startIndex = 0;
    if (lines.size() > 0 && lines[0].startsWith("timestamp", Qt::CaseInsensitive)) {
        startIndex = 1;
    }

    bool allValid = true;

    for (int i = startIndex; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.isEmpty()) continue;

        QStringList values = line.split(',', Qt::SkipEmptyParts);
        if (values.size() < 6) {
            qWarning() << "Invalid row (not enough columns):" << line;
            allValid = false;
            continue;
        }

        TradeData data;
        bool ok = false;

        // Парсим timestamp как число в миллисекундах
        qint64 timestampMs = values[0].trimmed().toLongLong(&ok);
        if (!ok) {
            qWarning() << "Invalid timestamp (not a number):" << values[0];
            allValid = false;
            continue;
        }
        data.timestamp = QDateTime::fromMSecsSinceEpoch(timestampMs, Qt::UTC);
        // data.timestamp = data.timestamp.toLocalTime(); // если хочешь локальное время

        // Парсим числовые поля
        data.open   = values[1].trimmed().toDouble(&ok); if (!ok) { allValid = false; continue; }
        data.high   = values[2].trimmed().toDouble(&ok); if (!ok) { allValid = false; continue; }
        data.low    = values[3].trimmed().toDouble(&ok); if (!ok) { allValid = false; continue; }
        data.close  = values[4].trimmed().toDouble(&ok); if (!ok) { allValid = false; continue; }
        data.volume = values[5].trimmed().toDouble(&ok); if (!ok) { allValid = false; continue; }

        tradeData_.append(data);
    }

    if (tradeData_.isEmpty()) {
        qWarning() << "No valid data parsed from CSV";
        return false;
    }

    qDebug() << "Successfully loaded" << tradeData_.size() << "records";
    return allValid;
}

const QVector<TradeData>& AlgoTradingController::getTradeData() const {
    return tradeData_;
}

int AlgoTradingController::getDataCount() const {
    return tradeData_.size();
}

}  // namespace s21
