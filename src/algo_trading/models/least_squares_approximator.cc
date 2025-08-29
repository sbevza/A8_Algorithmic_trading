#include "least_squares_approximator.h"

#include <stdexcept>
#include <algorithm>

namespace s21 {

LeastSquaresApproximator::LeastSquaresApproximator(const std::vector<SplinePoint>& points) {
  if (points.empty()) {
    throw std::invalid_argument("LeastSquaresApproximator: no points provided.");
  }

  auto sorted = points;
  std::sort(sorted.begin(), sorted.end(),
            [](const SplinePoint& a, const SplinePoint& b) { return a.x < b.x; });

  for (const auto& p : sorted) {
    x_vals_.push_back(p.x);
    y_vals_.push_back(p.y);
  }
}




}  // namespace s21
