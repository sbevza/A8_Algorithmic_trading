// src/tests/newton_interpolator_test.cc

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "models/newton_interpolator.h"

using namespace s21;

static bool almost_equal(double a, double b, double eps = 1e-9) {
  return std::abs(a - b) < eps;
}

class NewtonInterpolatorTest : public ::testing::Test {
 protected:
  std::vector<SplinePoint> points_small;   // x=[0,1,2,3], y=[1,3,7,13]
  std::vector<SplinePoint> points_linear;  // y = 2x

  void SetUp() override {
    points_small = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 7.0}, {3.0, 13.0}};
    points_linear = {{0.0, 0.0}, {1.0, 2.0}, {2.0, 4.0}, {3.0, 6.0}};
  }
};

// Тест: корректность построения для 4 точек (степень 3)
TEST_F(NewtonInterpolatorTest, Interpolate_ExactPolynomial) {
  NewtonInterpolator interp(points_small, 3);  // степень 3 → 4 точки

  // В узлах должно быть точно
  EXPECT_TRUE(almost_equal(interp.interpolate(0.0), 1.0));
  EXPECT_TRUE(almost_equal(interp.interpolate(1.0), 3.0));
  EXPECT_TRUE(almost_equal(interp.interpolate(2.0), 7.0));
  EXPECT_TRUE(almost_equal(interp.interpolate(3.0), 13.0));

  // Между узлами
  EXPECT_TRUE(almost_equal(interp.interpolate(0.5), 1.75));
  EXPECT_TRUE(
      almost_equal(interp.interpolate(1.5), 1.5 * 1.5 + 1.5 + 1, 1e-10));
  EXPECT_TRUE(
      almost_equal(interp.interpolate(2.5), 2.5 * 2.5 + 2.5 + 1, 1e-10));
}

// Тест: линейная функция, степень 1
TEST_F(NewtonInterpolatorTest, Interpolate_Linear) {
  const NewtonInterpolator interp(points_linear, 1);

  EXPECT_TRUE(almost_equal(interp.interpolate(0.0), 0.0));
  EXPECT_TRUE(almost_equal(interp.interpolate(1.0), 2.0));
  EXPECT_TRUE(almost_equal(interp.interpolate(0.5), 1.0));
  EXPECT_TRUE(almost_equal(interp.interpolate(2.5), 5.0));
}

// Тест: степень 0 — константа
TEST_F(NewtonInterpolatorTest, Interpolate_Constant) {
  NewtonInterpolator interp(points_small, 0);

  double value = interp.interpolate(100.0);
  EXPECT_TRUE(almost_equal(value, 1.0));
}

// Тест: выбор точек при степени 2 (нужно 3 точки)
TEST_F(NewtonInterpolatorTest, SelectPoints_Degree2) {
  NewtonInterpolator interp(points_small, 2);

  EXPECT_TRUE(almost_equal(interp.interpolate(0.0), 1.0));
  EXPECT_TRUE(almost_equal(interp.interpolate(3.0), 13.0));
}

// Тест: степень больше, чем точек — исключение
TEST_F(NewtonInterpolatorTest, DegreeTooHigh_Throws) {
  EXPECT_THROW(
      { NewtonInterpolator interp(points_small, 10); }, std::invalid_argument);
}

// Тест: пустой ввод — исключение
TEST_F(NewtonInterpolatorTest, EmptyPoints_Throws) {
  std::vector<SplinePoint> empty;
  EXPECT_THROW({ NewtonInterpolator interp(empty, 2); }, std::invalid_argument);
}

// Тест: одинаковые x — исключение
TEST_F(NewtonInterpolatorTest, DuplicateX_Throws) {
  std::vector<SplinePoint> bad = {{0, 0}, {0, 1}, {1, 2}};
  EXPECT_THROW({ NewtonInterpolator interp(bad, 2); }, std::invalid_argument);
}