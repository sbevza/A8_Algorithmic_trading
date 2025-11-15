// src/tests/least_squares_approximator_test.cc

#include "../algo_trading/models/least_squares_approximator.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

static bool almost_equal(double a, double b, double eps = 1e-6) {
  return std::abs(a - b) < eps;
}

class LeastSquaresApproximatorTest : public ::testing::Test {
 protected:
  std::vector<s21::TradeData> points_linear;     // y = 2x + 1
  std::vector<s21::TradeData> points_quadratic;  // y = x^2
  std::vector<s21::TradeData> points_constant;   // y = 5

  void SetUp() override {
    // Конструктор с 2 аргументами использует w = 1.0 по умолчанию
    points_linear = {
        {0.0, 1.1}, {1.0, 3.0}, {2.0, 5.1}, {3.0, 6.9}, {4.0, 9.0}};

    points_quadratic = {
        {-2.0, 4.0}, {-1.0, 1.0}, {0.0, 0.0}, {1.0, 1.0}, {2.0, 4.0}};

    points_constant = {{0.0, 5.0}, {1.0, 5.0}, {2.0, 5.0}, {3.0, 5.0}};
  }
};

// Тест: линейная аппроксимация (степень 1)
TEST_F(LeastSquaresApproximatorTest, Fit_Linear) {
  s21::LeastSquaresApproximator approx(points_linear);
  // Создаём вектор весов, размером с количество точек, заполненный 1.0
  std::vector<double> weights(points_linear.size(), 1.0);
  approx.fit(1, weights);

  EXPECT_TRUE(almost_equal(approx.predict(0.0), 1.0, 0.2));
  EXPECT_TRUE(almost_equal(approx.predict(1.0), 3.0, 0.2));
  EXPECT_TRUE(almost_equal(approx.predict(2.0), 5.0, 0.2));

  double y0 = approx.predict(0.0);
  double y2 = approx.predict(2.0);
  EXPECT_TRUE(y0 > 0.8 && y0 < 1.2);
  EXPECT_TRUE(y2 > 4.8 && y2 < 5.2);
}

// Тест: квадратичная аппроксимация (степень 2) для y = x^2
TEST_F(LeastSquaresApproximatorTest, Fit_Quadratic_Perfect) {
  s21::LeastSquaresApproximator approx(points_quadratic);
  // Создаём вектор весов, размером с количество точек, заполненный 1.0
  std::vector<double> weights(points_quadratic.size(), 1.0);
  approx.fit(2, weights);  // y = ax^2 + bx + c

  EXPECT_TRUE(almost_equal(approx.predict(-1.5), 2.25, 1e-5));
  EXPECT_TRUE(almost_equal(approx.predict(0.5), 0.25, 1e-5));
  EXPECT_TRUE(almost_equal(approx.predict(1.5), 2.25, 1e-5));

  EXPECT_TRUE(almost_equal(approx.predict(-2.0), 4.0));
  EXPECT_TRUE(almost_equal(approx.predict(0.0), 0.0));
}

// Тест: константная аппроксимация (степень 0)
TEST_F(LeastSquaresApproximatorTest, Fit_Constant) {
  s21::LeastSquaresApproximator approx(points_constant);
  // Создаём вектор весов, размером с количество точек, заполненный 1.0
  std::vector<double> weights(points_constant.size(), 1.0);
  approx.fit(0, weights);  // y = c

  for (double x : {0.0, 1.0, 2.0, 3.0, 4.0}) {
    EXPECT_TRUE(almost_equal(approx.predict(x), 5.0));
  }
}

// Тест: степень 0 на линейных данных → среднее
TEST_F(LeastSquaresApproximatorTest, Fit_Degree0_Mean) {
  s21::LeastSquaresApproximator approx(points_linear);
  // Создаём вектор весов, размером с количество точек, заполненный 1.0
  std::vector<double> weights(points_linear.size(), 1.0);
  approx.fit(0, weights);

  double sum_y = 0.0;
  for (const auto& p : points_linear) sum_y += p.close;
  double mean = sum_y / points_linear.size();

  EXPECT_TRUE(almost_equal(approx.predict(0.0), mean, 1e-10));
}

// Тест: степень больше, чем точек — исключение
TEST_F(LeastSquaresApproximatorTest, DegreeTooHigh_Throws) {
  s21::LeastSquaresApproximator approx(points_linear);
  // Создаём вектор весов, размером с количество точек, заполненный 1.0
  std::vector<double> weights(points_linear.size(), 1.0);
  EXPECT_THROW(approx.fit(10, weights), std::invalid_argument);
}

// Тест: пустой ввод — исключение
TEST_F(LeastSquaresApproximatorTest, EmptyPoints_Throws) {
  std::vector<s21::TradeData> empty;
  EXPECT_THROW({ s21::LeastSquaresApproximator approx(empty); },
               std::invalid_argument);
}

// Тест: предсказание до fit — поведение не определено, но не должно крашиться
TEST_F(LeastSquaresApproximatorTest, Predict_BeforeFit) {
  s21::LeastSquaresApproximator approx(points_linear);
  EXPECT_TRUE(almost_equal(approx.predict(1.0), 0.0));
}

// Тест: аналитическое решение для линейной регрессии
TEST_F(LeastSquaresApproximatorTest, Linear_Analytical_Check) {
  std::vector<s21::TradeData> perfect = {
      {0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};

  s21::LeastSquaresApproximator approx(perfect);
  // Создаём вектор весов, размером с количество точек, заполненный 1.0
  std::vector<double> weights(perfect.size(), 1.0);
  approx.fit(1, weights);

  EXPECT_TRUE(almost_equal(approx.predict(0.0), 1.0, 1e-5));
  EXPECT_TRUE(almost_equal(approx.predict(1.0), 3.0, 1e-5));
  EXPECT_TRUE(almost_equal(approx.predict(2.5), 6.0, 1e-5));
}