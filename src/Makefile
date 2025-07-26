.PHONY: all install open uninstall dist test gcov clean leak div

GCC = gcc -Wall -Werror -Wextra -std=c++17 

TEST_FLAGS = -lgtest -lgtest_main -lstdc++ -lm -g 
TEST_FILES = Tests/algo_trading_test.cc Tests/algo_trading_solver_test.cc Tests/algo_trading_generator_test.cc Tests/cave_test.cc algo_trading/models/algo_trading.cc algo_trading/models/algo_trading_solver.cc algo_trading/models/algo_trading_generator.cc
TEST = $(GCC) $(TEST_FILES) $(TEST_FLAGS) -o s21_test


GCOVFLAGS = -fprofile-arcs -ftest-coverage

OS = $(shell uname)

ifeq ($(OS), Darwin)
	TEST_LIBS = -lcheck -lgcov
	MOVE = mkdir build && mv algo_trading/algorithmic_trading.app build
	OPEN = cd build && open .
	LEAK = leaks -atExit -- ./s21_test 
	MISSMATCH = --ignore-errors mismatch
else
	TEST_LIBS = -lcheck -lsubunit -pthread -lm -lrt -lgcov
	MOVE = mkdir build && mv algo_trading/algorithmic_trading build
	OPEN = build/algorithmic_trading
	LEAK = valgrind --leak-check=full ./s21_test
endif

all: uninstall install

install:
	qmake6 -o algo_trading/Makefile algo_trading/algorithmic_trading.pro
	cd algo_trading/ && make 
	$(MOVE)
	cd algo_trading/ && make clean && rm -rf Makefile .qmake.stash
	$(OPEN)

open:
	$(OPEN)

uninstall:
	rm -rf build/

dist:
	mkdir algo_trading_dist/
	cp -a algo_trading Exemples Tests Makefile readme.txt  algo_trading_dist
	tar cvzf algo_trading.tgz algo_trading_dist/
	rm -rf algo_trading_dist/

test:
	$(TEST)
	./s21_test

gcov: 
	$(GCC) $(GCOVFLAGS) $(TEST_FILES) $(TEST_FLAGS) -o report.out
	./report.out
	lcov -t "gcov_report" -o gcov_report.info --no-external -c -d . $(MISSMATCH)
	genhtml -o report/ gcov_report.info
	rm -f *.gcno *.gcda *.info report.out *.gcov
	open ./report/index.html



clean: 
	find . \( -name '*.o' -o -name '*.a' -o -name '*.gcno' -o -name '*.gcda' -o -name '*.info' \) -exec rm -rf {} \;
	rm -rf report report_out report.out.dSYM s21_test.dSYM s21_test algo_trading.tgz view algo_trading.pro.*
	cd algo_trading/ && make clean && rm -rf Makefile .qmake.stash algo_trading.app

clang:
	find . -name '*.cc' -o -name '*.h' | xargs clang-format -n -style=Google

leak: 
	$(TEST)
	$(LEAK)

dvi:
	open readme.txt


