// src/algo_trading/models/file_reader.cc

#include "file_reader.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace s21 {

std::string FileReader::ReadFile(const std::string& path) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file '" << path << "'" << std::endl;
    return "";
  }

  std::ostringstream content;
  content << file.rdbuf();

  if (file.bad()) {
    std::cerr << "Error: Failed to read file '" << path << "'" << std::endl;
    return "";
  }

  return content.str();
}

bool FileReader::FileExists(const std::string& path) {
  const std::ifstream file(path);
  return file.good();
}

}  // namespace s21
