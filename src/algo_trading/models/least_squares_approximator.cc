// src/algo_trading/models/least_squares_approximator.cc

#include "least_squares_approximator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace s21 {

LeastSquaresApproximator::LeastSquaresApproximator(
    const std::vector<TradeData>& points)
    : degree_(0) {
  if (points.empty()) {
    throw std::invalid_argument(
        "LeastSquaresApproximator: no points provided.");
  }

  auto sorted = points;
  std::ranges::sort(sorted,
      [](const TradeData& a, const TradeData& b) { return a.timestamp < b.timestamp; });

  for (const auto& p : sorted) {
    x_vals_.push_back(p.timestamp);
    y_vals_.push_back(p.close);
  }
  coefficients_ = {0.0};
}

void LeastSquaresApproximator::fit(const int degree) {
  if (degree < 0) {
    throw std::runtime_error("Degree must be non-negative.");
  }
  if (degree >= static_cast<int>(x_vals_.size())) {
    throw std::invalid_argument("Degree too high for number of points.");
  }

  degree_ = degree;
  int n = static_cast<int>(x_vals_.size());
  int m = degree_ + 1;

  std::vector<std::vector<double>> A(m, std::vector<double>(m, 0.0));
  std::vector<double> B(m, 0.0);

  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < m; ++j) {
      double sum = 0.0;
      for (int k = 0; k < n; ++k) {
        sum += std::pow(x_vals_[k], i + j);
      }
      A[i][j] = sum;
    }
    double sum = 0.0;
    for (int k = 0; k < n; ++k) {
      sum += y_vals_[k] * std::pow(x_vals_[k], i);
    }
    B[i] = sum;
  }

  coefficients_.resize(m);
  for (int i = 0; i < m; ++i) coefficients_[i] = 0.0;

  for (int i = 0; i < m; ++i) {
    int max_row = i;
    for (int k = i + 1; k < m; ++k) {
      if (std::abs(A[k][i]) > std::abs(A[max_row][i])) {
        max_row = k;
      }
    }
    std::swap(A[i], A[max_row]);
    std::swap(B[i], B[max_row]);

    for (int k = i + 1; k < m; ++k) {
      double factor = A[k][i] / A[i][i];
      B[k] -= factor * B[i];
      for (int j = i; j < m; ++j) {
        A[k][j] -= factor * A[i][j];
      }
    }
  }

  for (int i = m - 1; i >= 0; --i) {
    coefficients_[i] = B[i];
    for (int j = i + 1; j < m; ++j) {
      coefficients_[i] -= A[i][j] * coefficients_[j];
    }
    coefficients_[i] /= A[i][i];
  }
}

double LeastSquaresApproximator::predict(const double x) const {
  if (coefficients_.empty() || degree_ < 0) {
    return 0.0;
  }

  double result = 0.0;
  double power = 1.0;
  for (int i = 0; i <= degree_; ++i) {
    result += coefficients_[i] * power;
    power *= x;
  }
  return result;
}

}  // namespace s21
