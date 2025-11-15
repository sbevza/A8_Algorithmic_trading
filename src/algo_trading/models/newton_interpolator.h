#ifndef A8_ALGORITHMIC_TRADING_1_NEWTONINTERPOLATOR_H
#define A8_ALGORITHMIC_TRADING_1_NEWTONINTERPOLATOR_H
#include <vector>

#include "trade_data.h"

namespace s21 {

class NewtonInterpolator {
 public:
  explicit NewtonInterpolator(const std::vector<TradeData>& points, int degree);
  [[nodiscard]] double interpolate(double x_val) const;
  [[nodiscard]] int degree() const { return degree_; }

 private:
  std::vector<double> x_vals_;
  std::vector<double> y_vals_;
  std::vector<double> coefficients_;
  int degree_;

  static std::vector<s21::TradeData> selectPointsUniformly(
      const std::vector<TradeData>& points, int n);
};

}  // namespace s21

#endif  // A8_ALGORITHMIC_TRADING_1_NEWTONINTERPOLATOR_H
