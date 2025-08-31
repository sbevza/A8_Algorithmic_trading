// src/tests/least_squares_approximator_test.cc

#include "models/least_squares_approximator.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace s21;

static bool almost_equal(double a, double b, double eps = 1e-6) {
  return std::abs(a - b) < eps;
}

class LeastSquaresApproximatorTest : public ::testing::Test {
 protected:
  std::vector<SplinePoint> points_linear;     // y = 2x + 1
  std::vector<SplinePoint> points_quadratic;  // y = x^2
  std::vector<SplinePoint> points_constant;   // y = 5

  void SetUp() override {
    points_linear = {
        {0.0, 1.1}, {1.0, 3.0}, {2.0, 5.1}, {3.0, 6.9}, {4.0, 9.0}};

    points_quadratic = {
        {-2.0, 4.0}, {-1.0, 1.0}, {0.0, 0.0}, {1.0, 1.0}, {2.0, 4.0}};

    points_constant = {{0.0, 5.0}, {1.0, 5.0}, {2.0, 5.0}, {3.0, 5.0}};
  }
};

// Тест: линейная аппроксимация (степень 1)
TEST_F(LeastSquaresApproximatorTest, Fit_Linear) {
  LeastSquaresApproximator approx(points_linear);
  approx.fit(1);

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
  LeastSquaresApproximator approx(points_quadratic);
  approx.fit(2);  // y = ax^2 + bx + c

  EXPECT_TRUE(almost_equal(approx.predict(-1.5), 2.25, 1e-5));
  EXPECT_TRUE(almost_equal(approx.predict(0.5), 0.25, 1e-5));
  EXPECT_TRUE(almost_equal(approx.predict(1.5), 2.25, 1e-5));

  EXPECT_TRUE(almost_equal(approx.predict(-2.0), 4.0));
  EXPECT_TRUE(almost_equal(approx.predict(0.0), 0.0));
}

// Тест: константная аппроксимация (степень 0)
TEST_F(LeastSquaresApproximatorTest, Fit_Constant) {
  LeastSquaresApproximator approx(points_constant);
  approx.fit(0);  // y = c

  for (double x : {0.0, 1.0, 2.0, 3.0, 4.0}) {
    EXPECT_TRUE(almost_equal(approx.predict(x), 5.0));
  }
}

// Тест: степень 0 на линейных данных → среднее
TEST_F(LeastSquaresApproximatorTest, Fit_Degree0_Mean) {
  LeastSquaresApproximator approx(points_linear);
  approx.fit(0);

  double sum_y = 0.0;
  for (const auto& p : points_linear) sum_y += p.y;
  double mean = sum_y / points_linear.size();

  EXPECT_TRUE(almost_equal(approx.predict(0.0), mean, 1e-10));
}

// Тест: степень больше, чем точек — исключение
TEST_F(LeastSquaresApproximatorTest, DegreeTooHigh_Throws) {
  LeastSquaresApproximator approx(points_linear);
  EXPECT_THROW(approx.fit(10), std::invalid_argument);
}

// Тест: пустой ввод — исключение
TEST_F(LeastSquaresApproximatorTest, EmptyPoints_Throws) {
  std::vector<SplinePoint> empty;
  EXPECT_THROW(
      { LeastSquaresApproximator approx(empty); }, std::invalid_argument);
}

// Тест: предсказание до fit — поведение не определено, но не должно крашиться
TEST_F(LeastSquaresApproximatorTest, Predict_BeforeFit) {
  LeastSquaresApproximator approx(points_linear);
  EXPECT_TRUE(almost_equal(approx.predict(1.0), 0.0));
}

// Тест: аналитическое решение для линейной регрессии
TEST_F(LeastSquaresApproximatorTest, Linear_Analytical_Check) {
  std::vector<SplinePoint> perfect = {
      {0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};

  LeastSquaresApproximator approx(perfect);
  approx.fit(1);

  EXPECT_TRUE(almost_equal(approx.predict(0.0), 1.0, 1e-5));
  EXPECT_TRUE(almost_equal(approx.predict(1.0), 3.0, 1e-5));
  EXPECT_TRUE(almost_equal(approx.predict(2.5), 6.0, 1e-5));
}