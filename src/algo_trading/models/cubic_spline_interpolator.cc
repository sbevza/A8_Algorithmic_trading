#include "cubic_spline_interpolator.h"

#include <algorithm>
#include <iostream>
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
  for (const auto& p : points) {
    xs_.push_back(p.x);
    ys_.push_back(p.y);
  }

  for (size_t i = 0; i < xs_.size() - 1; ++i) {
    if (xs_[i] >= xs_[i + 1]) {
      throw std::runtime_error("X-coordinates must be strictly increasing.");
    }
  }

  a_coeffs_.resize(points.size() - 1);
  b_coeffs_.resize(points.size() - 1);
  c_coeffs_.resize(points.size());  // M_i / 2
  d_coeffs_.resize(points.size() - 1);

  buildSpline();
}

void CubicSplineInterpolator::buildSpline() {
  size_t n = xs_.size();
  std::vector<double> h(n - 1);
  for (size_t i = 0; i < n - 1; ++i) {
    h[i] = xs_[i+1] - xs_[i];
  }

  std::vector<double> alpha(n - 1, 0.0);
  for (size_t i = 1; i < n - 1; ++i) {
    alpha[i] = 3.0 * (
        (ys_[i+1] - ys_[i]) / h[i] -
        (ys_[i] - ys_[i-1]) / h[i-1]
    );
  }

  // === 🔹 ОТЛАДКА 1: Вывод h и alpha ===
  std::cout << "DEBUG: h = ";
  for (double val : h) std::cout << val << " ";
  std::cout << std::endl;

  std::cout << "DEBUG: alpha = ";
  for (size_t i = 0; i < alpha.size(); ++i) {
    std::cout << "[" << i << "]=" << alpha[i] << " ";
  }
  std::cout << std::endl;
  // ====================================

  std::vector<double> l(n, 0.0), z(n, 0.0);

  l[0] = 1.0;
  z[0] = 0.0;

  for (size_t i = 1; i < n - 1; ++i) {
    double denom = 2.0 * (h[i-1] + h[i]) - h[i-1] * l[i-1];
    if (denom == 0) throw std::runtime_error("Singular system.");
    l[i] = h[i] / denom;
    z[i] = (alpha[i] - h[i-1] * z[i-1]) / denom;

    std::cout << "  i=" << i
                  << " denom=" << denom
                  << " l[" << i << "]=" << l[i]
                  << " z[" << i << "]=" << z[i]
                  << std::endl;
  }

  c_coeffs_[n-1] = 0.0;
  for (int i = n - 2; i >= 0; --i) {
    c_coeffs_[i] = z[i] - l[i] * c_coeffs_[i+1];
  }

  c_coeffs_[0] = 0.0;

  // === 🔹 ОТЛАДКА 3: Результат c_coeffs_ ===
  std::cout << "DEBUG: c_coeffs_ = ";
  for (size_t i = 0; i < c_coeffs_.size(); ++i) {
    std::cout << "[" << i << "]=" << c_coeffs_[i] << " ";
  }
  std::cout << std::endl;
  // ========================================

  for (size_t i = 0; i < n - 1; ++i) {
    a_coeffs_[i] = ys_[i];
    b_coeffs_[i] = (ys_[i+1] - ys_[i]) / h[i] -
                   h[i] * (2.0 * c_coeffs_[i] + c_coeffs_[i+1]) / 3.0;
    d_coeffs_[i] = (c_coeffs_[i+1] - c_coeffs_[i]) / (3.0 * h[i]);
  }

}

double CubicSplineInterpolator::interpolate(double x_val) const {
  if (x_val < xs_.front() || x_val > xs_.back()) {
    // return std::numeric_limits<double>::quiet_NaN();
    throw std::runtime_error("Interpolation point x is out of data range.");
  }

  auto it = std::upper_bound(xs_.begin(), xs_.end(), x_val);
  size_t i = std::distance(xs_.begin(), it) - 1;

  if (i >= xs_.size() - 1) {
    i = xs_.size() - 2;
  }
  if (x_val == xs_.back() && xs_.size() > 1) {
    i = xs_.size() - 2;
  }

  double dx = x_val - xs_[i];

  // 🔹 Отладка
  std::cout << "interpolate(" << x_val << ") -> i=" << i
            << " x[i]=" << xs_[i] << " dx=" << dx
            << " a=" << a_coeffs_[i]
            << " b=" << b_coeffs_[i]
            << " c=" << c_coeffs_[i]
            << " d=" << d_coeffs_[i]
            << std::endl;

  double result = a_coeffs_[i] + b_coeffs_[i] * dx + c_coeffs_[i] * dx * dx + d_coeffs_[i] * dx * dx * dx;
  std::cout << " -> result = " << result << std::endl;
  return result;
  // return a_coeffs_[i] + b_coeffs_[i] * dx + c_coeffs_[i] * dx * dx +
  //        d_coeffs_[i] * dx * dx * dx;
}
}  // namespace s21