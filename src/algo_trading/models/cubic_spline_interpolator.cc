#include "cubic_spline_interpolator.h"

#include <algorithm>
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


  a_coeffs_.resize(points.size() - 1);  // a_i = y_i
  b_coeffs_.resize(points.size() - 1);
  c_coeffs_.resize(points.size());  // M_i / 2
  d_coeffs_.resize(points.size() - 1);

  buildSpline();
}

void CubicSplineInterpolator::buildSpline() {
  size_t n = xs_.size();
  std::vector<double> h(n - 1);
  std::vector<double> alpha(n - 1);

  for (size_t i = 0; i < n - 1; ++i) {
    h[i] = xs_[i + 1] - xs_[i];
    if (h[i] <= 0) {
      throw std::runtime_error("Invalid x-coordinates: h must be positive.");
    }
  }

  for (size_t i = 1; i < n - 1; ++i) {
    alpha[i] =
        3.0 * ((ys_[i + 1] - ys_[i]) / h[i] - (ys_[i] - ys_[i - 1]) / h[i - 1]);
  }


  std::vector<double> l(n);
  std::vector<double> mu(n);
  std::vector<double> z(n);


  l[0] = 1.0;
  mu[0] = 0.0;
  z[0] = 0.0;

  for (size_t i = 1; i < n - 1; ++i) {
    double den = 2.0 * (xs_[i + 1] - xs_[i - 1]) - h[i - 1] * mu[i - 1];
    if (den == 0) {
      throw std::runtime_error(
          "Error in spline calculation: division by zero (den).");
    }
    l[i] = h[i] / den;
    mu[i] = alpha[i] - h[i - 1] * z[i - 1];
    z[i] = mu[i] / den;
  }

  // Обратный проход
  c_coeffs_[n - 1] =
      0.0;
  for (int i = n - 2; i >= 0; --i) {
    c_coeffs_[i] = z[i] - l[i] * c_coeffs_[i + 1];
  }

  for (size_t i = 0; i < n - 1; ++i) {
    a_coeffs_[i] = ys_[i];

    if (h[i] == 0) {
      throw std::runtime_error("Error in spline calculation: h[i] is zero.");
    }

    b_coeffs_[i] = (ys_[i + 1] - ys_[i]) / h[i] -
                   h[i] * (c_coeffs_[i + 1] + 2.0 * c_coeffs_[i]) / 3.0;
    d_coeffs_[i] = (c_coeffs_[i + 1] - c_coeffs_[i]) / (3.0 * h[i]);

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
    i = xs_.size() -
        2;
  }

  double dx = x_val - xs_[i];


  return a_coeffs_[i] + b_coeffs_[i] * dx + c_coeffs_[i] / 2.0 * dx * dx +
         d_coeffs_[i] * dx * dx * dx;
}
}  // namespace s21