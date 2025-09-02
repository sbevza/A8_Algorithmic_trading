// src/algo_trading/models/csv_parser.cc

#include "csv_parser.h"

#include <QDebug>
#include <QStringList>
#include <QTextStream>
#include <cmath>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <QDateTime>

#include "trade_data.h"

namespace s21 {

std::vector<std::string> CsvParser::Split(const std::string& s,
                                          char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter)) {
    size_t start = token.find_first_not_of(" \t\r\n");
    size_t end = token.find_last_not_of(" \t\r\n");
    if (start != std::string::npos && end != std::string::npos) {
      token = token.substr(start, end - start + 1);
    } else {
      token.clear();
    }
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }
  return tokens;
}

QVector<TradeData> CsvParser::Parse(const std::string& content) {
  error_message_.clear();
  QVector<TradeData> data;
  data.clear();

  if (content.empty()) {
    error_message_ = "Input content is empty.";
    return data;
  }

  std::istringstream stream(content);
  std::string line;
  std::vector<std::string> lines;

  while (std::getline(stream, line)) {
    std::string trimmed = line;
    auto start = trimmed.find_first_not_of(" \t\r\n");
    if (start != std::string::npos) {
      lines.push_back(line);
    }
  }

  if (lines.empty()) {
    error_message_ = "File is empty.";
    return data;
  }

  size_t start_index = 0;
  if (IsHeader(lines[0])) {
    start_index = 1;
  }

  for (size_t i = start_index; i < lines.size(); ++i) {
    std::string line = lines[i];
    auto parts = Split(line, ',');

    if (parts.size() < 2) {
      error_message_ =
          "Error on line " + std::to_string(i + 1) + ": insufficient data.";
      std::cerr << error_message_ << " Line: " << line << std::endl;
      data.clear();
      return data;
    }

    std::istringstream is(parts[0]);
    int year, month, day;
    char dash;
    is >> year >> dash >> month >> dash >> day;
    std::chrono::year_month_day ymd = std::chrono::year{year} / month / day;;
    if (is.fail() || !ymd.ok()) {
      error_message_ = "Error on line " + std::to_string(i + 1) + ": invalid date.";
      std::cerr << error_message_ << " Date: " << parts[0] << std::endl;
      data.clear();
      return data;
    }
    auto time_point = std::chrono::sys_days{ymd};
    auto duration = time_point.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto timestamp = static_cast<double>(seconds);

    bool price_ok = false;
    double price = 0.0;
    try {
      price = std::stod(parts[1]);
      if (std::isnan(price) || std::isinf(price)) throw std::invalid_argument("nan or inf");
      price_ok = true;
    } catch (...) {
      price_ok = false;
    }

    if (!price_ok) {
      error_message_ = "Error on line " + std::to_string(i + 1) + ": invalid price format.";
      std::cerr << error_message_ << " Price: " << parts[1] << std::endl;
      data.clear();
      return data;
    }

    double weight = 1.0;
    if (parts.size() >= 3 && !parts[2].empty()) {
      try {
        double parsed_weight = std::stod(parts[2]);
        if (!std::isnan(parsed_weight) && !std::isinf(parsed_weight) && parsed_weight >= 0) {
          weight = parsed_weight;
        } else {
          std::cout << "Warning: Invalid weight on line " << (i + 1)
                    << ". Using 1.0. Weight: " << parts[2] << std::endl;
        }
      } catch (...) {
        std::cout << "Warning: Invalid weight on line " << (i + 1)
                  << ". Using 1.0. Weight: " << parts[2] << std::endl;
      }
    }

    data.append(TradeData(timestamp, price, weight));
  }

  error_message_ = "No valid data lines were parsed.";
  if (data.isEmpty()) {
  }

  return data;
}

bool CsvParser::IsHeader(const std::string& line) {
  std::string prefix;
  if (line.length() > 10) {
    prefix = line.substr(0, 10);
  } else {
    prefix = line;
  }

  std::string lower;
  for (char c : prefix) {
    lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }

  return lower.find("date") != std::string::npos ||
         lower.find("time") != std::string::npos ||
         lower.find("price") != std::string::npos ||
         lower.find("value") != std::string::npos ||
         lower.find("weight") != std::string::npos ||
         lower.find("close") != std::string::npos;
}

std::string CsvParser::GetError() const { return error_message_; }

std::chrono::year_month_day parse_date(const std::string& s) {
  std::istringstream ss(s);
  int year, month, day;
  char dash;
  ss >> year >> dash >> month >> dash >> day;
  return std::chrono::year{year} / month / day;

}

}  // namespace s21
