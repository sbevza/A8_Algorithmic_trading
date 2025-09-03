#ifndef SRC_MODELS_CSV_PARSER_H_
#define SRC_MODELS_CSV_PARSER_H_


#include "trade_data.h"

namespace s21 {

class CsvParser {
 public:
  std::vector<TradeData> Parse(const std::string& content);
  [[nodiscard]] std::string GetError() const;

 private:
  std::string error_message_;
  static bool IsHeader(const std::string& line);
  static std::vector<std::string> Split(const std::string& s, char delimiter);
};

}  // namespace s21

#endif  // SRC_MODELS_CSV_PARSER_H_
