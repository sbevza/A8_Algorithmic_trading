#ifndef SRC_MODELS_CSV_PARSER_H_
#define SRC_MODELS_CSV_PARSER_H_

#include <QString>
#include <QVector>
#include "trade_data.h"

namespace s21 {

class CsvParser {
public:
    QVector<TradeData> Parse(const QString& content);
    QString GetError() const;

private:
    QString error_message_;
    bool IsHeader(const QString& line) const;
};

}  // namespace s21

#endif  // SRC_MODELS_CSV_PARSER_H_
