# 将tester.cpp与Pokemon.cpp和BattleController.cpp编译成output
# 通过make clean清除output
# 通过make run运行output

output: tester.o Pokemon.o BattleController.o
	g++ tester.o Pokemon.o BattleController.o -o output


.PHONY: clean
clean:
	rm *.o output


