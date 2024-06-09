#include <iostream>
#include <thread>

int main() {
  int counter = 0;

  //表面上不会冲突，但会被拆解为多条微指令
  std::thread t1([&] {
    for (int i = 0; i < 10000; i++) {
      counter += 1;
    }
  });

  std::thread t2([&] {
    for (int i = 0; i < 10000; i++) {
      counter += 1;
    }
  });

  t1.join();
  t2.join();

  std::cout << counter << std::endl;

  return 0;
}
