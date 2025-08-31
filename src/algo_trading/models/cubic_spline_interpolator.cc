// src/algo_trading/models/cubic_spline_interpolator.cc

#include "cubic_spline_interpolator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace s21 {

CubicSplineInterpolator::CubicSplineInterpolator(
    const std::vector<SplinePoint>& points) {
  if (points.size() < 2) {
    throw std::runtime_error(
        "CubicSplineInterpolator requires at least 2 points.");
  }

  xs_.reserve(points.size());
  ys_.reserve(points.size());
  for (const auto& [x, y] : points) {
    xs_.push_back(x);
    ys_.push_back(y);
  }

  for (size_t i = 0; i < xs_.size() - 1; ++i) {
    if (xs_[i] >= xs_[i + 1]) {
      throw std::runtime_error("X-coordinates must be strictly increasing.");
    }
  }

  a_coeffs_.resize(points.size() - 1);
  b_coeffs_.resize(points.size() - 1);
  c_coeffs_.resize(points.size());
  d_coeffs_.resize(points.size() - 1);

  buildSpline();
}

void CubicSplineInterpolator::buildSpline() {
  size_t n = xs_.size();
  std::vector<double> h(n - 1);
  for (size_t i = 0; i < n - 1; ++i) {
    h[i] = xs_[i + 1] - xs_[i];
  }

  std::vector<double> alpha(n - 1, 0.0);
  for (size_t i = 1; i < n - 1; ++i) {
    alpha[i] =
        3.0 * ((ys_[i + 1] - ys_[i]) / h[i] - (ys_[i] - ys_[i - 1]) / h[i - 1]);
  }

  std::vector<double> l(n, 0.0), z(n, 0.0);

  l[0] = 1.0;
  z[0] = 0.0;

  for (size_t i = 1; i < n - 1; ++i) {
    const double denom = 2.0 * (h[i - 1] + h[i]) - h[i - 1] * l[i - 1];
    if (denom == 0) throw std::runtime_error("Singular system.");
    l[i] = h[i] / denom;
    z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / denom;
  }

  c_coeffs_[n - 1] = 0.0;
  for (size_t i = n - 2; i > 0; --i) {
    c_coeffs_[i] = z[i] - l[i] * c_coeffs_[i + 1];
  }

  c_coeffs_[0] = 0.0;

  for (size_t i = 0; i < n - 1; ++i) {
    a_coeffs_[i] = ys_[i];
    b_coeffs_[i] = (ys_[i + 1] - ys_[i]) / h[i] -
                   h[i] * (2.0 * c_coeffs_[i] + c_coeffs_[i + 1]) / 3.0;
    d_coeffs_[i] = (c_coeffs_[i + 1] - c_coeffs_[i]) / (3.0 * h[i]);
  }
}

double CubicSplineInterpolator::interpolate(const double x_val) const {
  if (!std::isfinite(x_val)) {
    throw std::runtime_error("Interpolation point x is not finite.");
  }

  if (x_val < xs_.front() || x_val > xs_.back()) {
    throw std::runtime_error("Interpolation point x is out of data range.");
  }

  const auto it = std::ranges::upper_bound(xs_, x_val);
  size_t i = std::distance(xs_.begin(), it) - 1;

  if (i >= xs_.size() - 1) {
    i = xs_.size() - 2;
  }
  // if (x_val == xs_.back() && xs_.size() > 1) {
  //   i = xs_.size() - 2;
  // }

  const double dx = x_val - xs_[i];
  return a_coeffs_[i] + b_coeffs_[i] * dx + c_coeffs_[i] * dx * dx +
         d_coeffs_[i] * dx * dx * dx;
}
}  // namespace s21