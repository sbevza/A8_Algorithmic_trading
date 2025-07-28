cmake_minimum_required(VERSION 3.23)
project(A8_Algorithmic_trading_1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets OpenGLWidgets REQUIRED)
#find_package(GTest REQUIRED)

include_directories(src/algo_trading/controllers)
include_directories(src/algo_trading/models)
include_directories(src/algo_trading/views)

add_executable(A8_Algorithmic_trading_1
#        src/main.cc
        src/algo_trading/main.cc
        src/algo_trading/views/mainwindow.cc
        src/algo_trading/views/mainwindow.h
        src/algo_trading/algorithmic_trading.pro
        src/algo_trading/models/csv_parser.cc
        src/algo_trading/models/trade_data.cc
        src/algo_trading/models/csv_parser.h
        src/algo_trading/models/trade_data.h
        src/algo_trading/controllers/algo_trading_controller.cc
        src/algo_trading/controllers/algo_trading_controller.h

)

target_link_libraries(A8_Algorithmic_trading_1
        m Qt6::Core Qt6::Widgets Qt6::OpenGLWidgets gtest gtest_main)

#add_executable(A8_Algorithmic_trading_Tests
#        src/Tests/maze_test.cc
#        src/Tests/cave_test.cc
#        src/Tests/maze_solver_test.cc
#        src/Tests/maze_generator_test.cc
#        src/Maze/models/maze.cc
#        src/Maze/models/maze_solver.cc
#        src/Maze/models/maze_generator.cc
#        src/Maze/controllers/maze_controller.cc
#        src/Maze/controllers/maze_controller.h
#        src/Tests/test.cc
#)
#
#target_include_directories(A1_Maze_Tests PRIVATE ${GTEST_INCLUDE_DIRS})
#target_link_libraries(A1_Maze_Tests ${GTEST_LIBRARIES} Qt6::Core Qt6::Widgets Qt6::OpenGLWidgets pthread)
#
#enable_testing()
#add_test(NAME maze_solver_test COMMAND A1_Maze_Tests)
