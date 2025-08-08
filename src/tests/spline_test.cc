// tests/spline_test.cc
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "algo_trading/models/cubic_spline_interpolator.h"

using namespace s21;

struct SplinePoint {
  double x, y;
};

bool almost_equal(double a, double b, double eps = 1e-9) {
  return std::abs(a - b) < eps;
}

class CubicSplineTest : public ::testing::Test {
 protected:
  std::vector<s21::SplinePoint> points_linear;
  std::vector<s21::SplinePoint> points_quadratic;
  std::vector<s21::SplinePoint> points_cubic;

  void SetUp() override {
    // Линейная: y = 2x
    points_linear = {{0.0, 0.0}, {1.0, 2.0}, {2.0, 4.0}, {3.0, 6.0}};

    // Квадратичная: y = x^2
    points_quadratic = {{0.0, 0.0}, {1.0, 1.0}, {2.0, 4.0}, {3.0, 9.0}};

    // Кубическая: y = x^3
    points_cubic = {{0.0, 0.0}, {1.0, 1.0}, {2.0, 8.0}, {3.0, 27.0}};
  }
};

// Тест: минимальное количество точек
TEST_F(CubicSplineTest, Constructor_MinimumPoints) {
  std::vector<s21::SplinePoint> two_points = {{0.0, 0.0}, {1.0, 1.0}};
  EXPECT_NO_THROW({ CubicSplineInterpolator spline(two_points); });
}

// Тест: меньше двух точек — ошибка
TEST_F(CubicSplineTest, Constructor_InsufficientPoints) {
  std::vector<s21::SplinePoint> one_point = {{0.0, 0.0}};
  EXPECT_THROW(
      { CubicSplineInterpolator spline(one_point); }, std::runtime_error);
}

// Тест: x не строго возрастают — ошибка
TEST_F(CubicSplineTest, Constructor_NonIncreasingX) {
  std::vector<s21::SplinePoint> bad_x = {{0.0, 0.0}, {1.0, 1.0}, {1.0, 2.0}};
  EXPECT_THROW({ CubicSplineInterpolator spline(bad_x); }, std::runtime_error);

  std::vector<s21::SplinePoint> decreasing = {{2.0, 0.0}, {1.0, 1.0}};
  EXPECT_THROW(
      { CubicSplineInterpolator spline(decreasing); }, std::runtime_error);
}

// Тест: линейная функция — сплайн должен быть линейным
TEST_F(CubicSplineTest, Interpolate_LinearFunction) {
  CubicSplineInterpolator spline(points_linear);

  EXPECT_TRUE(almost_equal(spline.interpolate(0.0), 0.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(0.5), 1.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(1.0), 2.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(2.5), 5.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(3.0), 6.0));
}

// Тест: квадратичная функция y = x^2
TEST_F(CubicSplineTest, Interpolate_QuadraticFunction) {
  CubicSplineInterpolator spline(points_quadratic);

  EXPECT_TRUE(almost_equal(spline.interpolate(0.0), 0.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(1.0), 1.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(2.0), 4.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(3.0), 9.0));

  // Интерполяция между узлами
  EXPECT_NEAR(spline.interpolate(0.5), 0.25, 0.1);  // 0.295 → OK
  EXPECT_NEAR(spline.interpolate(1.5), 2.25, 0.2);
  EXPECT_NEAR(spline.interpolate(2.5), 6.25, 0.2);
}

// Тест: кубическая функция y = x^3
TEST_F(CubicSplineTest, Interpolate_CubicFunction) {
  CubicSplineInterpolator spline(points_cubic);

  EXPECT_TRUE(almost_equal(spline.interpolate(0.0), 0.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(1.0), 1.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(2.0), 8.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(3.0), 27.0));

  // Между узлами
  EXPECT_TRUE(almost_equal(spline.interpolate(0.5), 0.125, 0.05));
  EXPECT_TRUE(almost_equal(spline.interpolate(1.5), 3.375, 0.4));
  EXPECT_TRUE(almost_equal(spline.interpolate(2.5), 15.625, 1.0));
}

// Только в узлах
TEST_F(CubicSplineTest, InterpolationAtNodes) {
  CubicSplineInterpolator spline(points_quadratic);

  EXPECT_NEAR(spline.interpolate(0.0), 0.0, 1e-10);
  EXPECT_NEAR(spline.interpolate(1.0), 1.0, 1e-10);
  EXPECT_NEAR(spline.interpolate(2.0), 4.0, 1e-10);
  EXPECT_NEAR(spline.interpolate(3.0), 9.0, 1e-10);
}

TEST_F(CubicSplineTest, DebugInterpolation) {
  CubicSplineInterpolator spline(points_quadratic);
  std::cout << "interpolate(0.5) = " << spline.interpolate(0.5) << std::endl;
  std::cout << "interpolate(1.0) = " << spline.interpolate(1.0) << std::endl;
}

// Тест: выход за границы — исключение
TEST_F(CubicSplineTest, Interpolate_OutOfRange) {
  CubicSplineInterpolator spline(points_quadratic);

  EXPECT_THROW(spline.interpolate(-0.1), std::runtime_error);
  EXPECT_THROW(spline.interpolate(3.1), std::runtime_error);
}

// Тест: интерполяция на границе (в последней точке)
TEST_F(CubicSplineTest, Interpolate_AtBoundaries) {
  CubicSplineInterpolator spline(points_quadratic);

  EXPECT_TRUE(almost_equal(spline.interpolate(0.0), 0.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(3.0), 9.0));
}

// Тест: две точки — линейная интерполяция
TEST_F(CubicSplineTest, TwoPoints_Linear) {
  std::vector<s21::SplinePoint> two = {{0.0, 0.0}, {2.0, 4.0}};
  CubicSplineInterpolator spline(two);

  EXPECT_TRUE(almost_equal(spline.interpolate(0.0), 0.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(1.0), 2.0));
  EXPECT_TRUE(almost_equal(spline.interpolate(2.0), 4.0));
}