#ifndef CUBIC_SPLINE_INTERPOLATOR_H
#define CUBIC_SPLINE_INTERPOLATOR_H
#include <vector>

namespace s21 {

struct SplinePoint {
  double x;
  double y;
};

class CubicSplineInterpolator {
public:
  explicit CubicSplineInterpolator(const std::vector<SplinePoint>& points);
  [[nodiscard]] double interpolate(double x_val) const;

private:
  std::vector<double> xs_;
  std::vector<double> ys_;
  void buildSpline();
};

}  // namespace s21

#endif  // CUBIC_SPLINE_INTERPOLATOR_H
