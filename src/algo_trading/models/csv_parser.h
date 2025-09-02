#ifndef SRC_MODELS_CSV_PARSER_H_
#define SRC_MODELS_CSV_PARSER_H_

#include <QVector>

#include "trade_data.h"

namespace s21 {

class CsvParser {
 public:
  QVector<TradeData> Parse(const std::string& content);
  [[nodiscard]] std::string GetError() const;

 private:
  std::string error_message_;
  static bool IsHeader(const std::string& line);
  static std::vector<std::string> Split(const std::string& s, char delimiter);
  // static bool ParseDate(const std::string& date_str, std::tm& tm);
};

}  // namespace s21

#endif  // SRC_MODELS_CSV_PARSER_H_
