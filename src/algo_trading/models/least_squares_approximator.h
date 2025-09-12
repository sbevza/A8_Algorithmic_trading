#ifndef A8_ALGORITHMIC_TRADING_1_LEAST_SQUARES_APPROXIMATOR_H
#define A8_ALGORITHMIC_TRADING_1_LEAST_SQUARES_APPROXIMATOR_H
#include <vector>

#include "trade_data.h"

namespace s21 {

class LeastSquaresApproximator {
 public:
  explicit LeastSquaresApproximator(const std::vector<TradeData>& points);

  void fit(int degree, const std::vector<double>& weights);
  [[nodiscard]] double predict(double x) const;

 private:
  std::vector<double> x_vals_;
  std::vector<double> y_vals_;
  // std::vector<double> weights_;
  std::vector<double> coefficients_;
  int degree_ = 1;
};

}  // namespace s21

#endif  // A8_ALGORITHMIC_TRADING_1_LEAST_SQUARES_APPROXIMATOR_H
