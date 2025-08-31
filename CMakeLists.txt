cmake_minimum_required(VERSION 3.23)
project(A8_Algorithmic_trading_1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets PrintSupport REQUIRED)
find_package(GTest REQUIRED)

add_executable(A8_Algorithmic_trading_1
        src/algo_trading/main.cc
        src/algo_trading/qcustomplot/qcustomplot.cc
        src/algo_trading/views/mainwindow.cc
        src/algo_trading/views/mainwindow.h

        src/algo_trading/controllers/algo_trading_controller.cc
        src/algo_trading/controllers/algo_trading_controller.h

        src/algo_trading/models/csv_parser.cc
        src/algo_trading/models/trade_data.cc
        src/algo_trading/models/csv_parser.h
        src/algo_trading/models/trade_data.h
        src/algo_trading/models/cubic_spline_interpolator.cc
        src/algo_trading/models/cubic_spline_interpolator.h
        src/algo_trading/models/newton_interpolator.cc
        src/algo_trading/models/newton_interpolator.h
        src/algo_trading/models/least_squares_approximator.cc
        src/algo_trading/models/least_squares_approximator.h
)

target_link_libraries(A8_Algorithmic_trading_1
        Qt6::Core
        Qt6::Widgets
        Qt6::PrintSupport
        m
)

target_include_directories(A8_Algorithmic_trading_1 PRIVATE
        ${CMAKE_SOURCE_DIR}/src/algo_trading
)


add_executable(A8_Algorithmic_trading_Tests
        src/algo_trading/models/cubic_spline_interpolator.cc
        src/algo_trading/models/newton_interpolator.cc
        src/algo_trading/models/least_squares_approximator.cc
        src/tests/spline_test.cc
        src/tests/newton_test.cc
        src/tests/least_squares_approximator_test.cc
)

set_target_properties(A8_Algorithmic_trading_Tests PROPERTIES
        AUTOMOC OFF
        AUTOUIC OFF
)

target_link_libraries(A8_Algorithmic_trading_Tests
        GTest::gtest
        GTest::gtest_main
        m
)

target_include_directories(A8_Algorithmic_trading_Tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/algo_trading
)

include(GoogleTest)

enable_testing()

gtest_discover_tests(A8_Algorithmic_trading_Tests
        PROPERTIES
        ENVIRONMENT "GTEST_COLOR=1"
)
