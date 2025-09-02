// src/algo_trading/models/file_reader.h

#ifndef A8_ALGORITHMIC_TRADING_1_FILE_READER_H
#define A8_ALGORITHMIC_TRADING_1_FILE_READER_H
#include <string>

namespace s21 {

class FileReader {
 public:
  static std::string ReadFile(const std::string& path);
  static bool FileExists(const std::string& path);
};

}  // namespace s21

#endif  // A8_ALGORITHMIC_TRADING_1_FILE_READER_H
