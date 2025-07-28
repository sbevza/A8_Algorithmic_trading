// csv_parser.cc
#include "csv_parser.h"
#include "trade_data.h" // ✅ Включаем напрямую

#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <cmath>

namespace s21 {

QVector<TradeData> CsvParser::Parse(const QString& content) {
    error_message_.clear();
    QVector<TradeData> data;
    data.clear();

    QStringList lines = content.trimmed().split('\n', Qt::SkipEmptyParts);

    if (lines.isEmpty()) {
        error_message_ = "Файл пустой.";
        return data;
    }

    int start_index = 0;
    if (IsHeader(lines[0])) {
        start_index = 1;
    }

    const QString kDateFormat = "yyyy-MM-dd";

    for (int i = start_index; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(',', Qt::SkipEmptyParts);

        if (parts.size() < 2) {
            error_message_ = QString("Ошибка в строке %1: недостаточно данных.").arg(i + 1);
            qDebug() << error_message_ << "Строка:" << line;
            data.clear();
            return data;
        }

        QDateTime timestamp = QDateTime::fromString(parts[0].trimmed(), kDateFormat);
        if (!timestamp.isValid()) {
            error_message_ = QString("Ошибка в строке %1: неверный формат даты.").arg(i + 1);
            qDebug() << error_message_ << "Дата:" << parts[0];
            data.clear();
            return data;
        }

        bool price_ok = false;
        double price = parts[1].trimmed().toDouble(&price_ok);
        if (!price_ok || std::isnan(price)) {
            error_message_ = QString("Ошибка в строке %1: неверный формат цены.").arg(i + 1);
            qDebug() << error_message_ << "Цена:" << parts[1];
            data.clear();
            return data;
        }

        double weight = 1.0;
        if (parts.size() >= 3 && !parts[2].trimmed().isEmpty()) {
            bool weight_ok = false;
            weight = parts[2].trimmed().toDouble(&weight_ok);
            if (!weight_ok || std::isnan(weight) || weight < 0) {
                qWarning() << QString("Предупреждение: Неверный вес в строке %1. Используется 1.0.").arg(i + 1);
                weight = 1.0;
            }
        }

        data.append(TradeData(timestamp, price, weight));
    }

    if (data.isEmpty()) {
        error_message_ = "Не удалось распарсить ни одной строки с данными.";
    }

    return data;
}

bool CsvParser::IsHeader(const QString& line) const {
    QString first_part = line.left(10).toLower();
    return first_part.contains("date") || first_part.contains("time") ||
           first_part.contains("price") || first_part.contains("value") ||
           first_part.contains("weight") || first_part.contains("close");
}

QString CsvParser::GetError() const {
    return error_message_;
}

}  // namespace s21
