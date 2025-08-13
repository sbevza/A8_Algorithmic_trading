// src/algo_trading/models/newton_interpolator.cc

#include "newton_interpolator.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace s21 {

NewtonInterpolator::NewtonInterpolator(const std::vector<SplinePoint>& points,
                                       const int degree)
    : degree_(degree) {
  if (points.empty()) {
    throw std::invalid_argument("NewtonInterpolator: no points provided.");
  }

  if (degree < 0) {
    throw std::invalid_argument(
        "NewtonInterpolator: degree cannot be negative.");
  }

  const int required_points = degree + 1;
  int total_available = static_cast<int>(points.size());

  if (required_points > total_available) {
    throw std::invalid_argument(
        "NewtonInterpolator: degree too high. Not enough points available.");
  }

  auto selected = selectPointsUniformly(points, required_points);

  std::ranges::sort(selected,
      [](const SplinePoint& a, const SplinePoint& b) { return a.x < b.x; });

  for (size_t i = 0; i < selected.size() - 1; ++i) {
    if (selected[i].x == selected[i + 1].x) {
      throw std::invalid_argument(
          "NewtonInterpolator: duplicate x-values in selected points.");
    }
  }

  x_vals_.clear();
  y_vals_.clear();
  for (const auto& p : selected) {
    x_vals_.push_back(p.x);
    y_vals_.push_back(p.y);
  }

  const size_t n = x_vals_.size();

  std::vector<std::vector<double>> diff_table(n, std::vector<double>(n, 0.0));

  for (size_t i = 0; i < n; ++i) {
    diff_table[i][0] = y_vals_[i];
  }

  for (size_t j = 1; j < n; ++j) {
    for (size_t i = 0; i < n - j; ++i) {
      const double dx = x_vals_[i + j] - x_vals_[i];
      if (dx == 0.0) {
        throw std::runtime_error(
            "NewtonInterpolator: division by zero in divided differences.");
      }
      diff_table[i][j] = (diff_table[i + 1][j - 1] - diff_table[i][j - 1]) / dx;
    }
  }

  coefficients_.resize(n);
  for (size_t i = 0; i < n; ++i) {
    coefficients_[i] = diff_table[0][i];
  }
}

double NewtonInterpolator::interpolate(const double x_val) const {
  if (coefficients_.empty()) return 0.0;

  double result = coefficients_[0];
  double term = 1.0;

  for (size_t i = 1; i < coefficients_.size(); ++i) {
    term *= (x_val - x_vals_[i - 1]);
    result += coefficients_[i] * term;
  }

  return result;
}

std::vector<SplinePoint> NewtonInterpolator::selectPointsUniformly(
    const std::vector<SplinePoint>& points, const int n) {
  if (n >= static_cast<int>(points.size())) {
    return points;
  }

  std::vector<SplinePoint> result;
  result.reserve(n);

  const double x_min = points.front().x;
  const double x_max = points.back().x;

  for (int i = 0; i < n; ++i) {
    const double t = (n == 1) ? 0.0 : static_cast<double>(i) / (n - 1);
    double target_x = x_min + t * (x_max - x_min);

    auto it = std::lower_bound(
        points.begin(), points.end(), target_x,
        [](const SplinePoint& a, double x) { return a.x < x; });

    if (it == points.end()) {
      result.push_back(points.back());
    } else if (it == points.begin()) {
      result.push_back(points.front());
    } else {
      const double diff_next = std::abs(it->x - target_x);
      const double diff_prev = std::abs((it - 1)->x - target_x);
      result.push_back(diff_next < diff_prev ? *it : *(it - 1));
    }
  }

  return result;
}

}  // namespace s21
