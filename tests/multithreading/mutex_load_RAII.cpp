#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

//将锁的持有视为资源
int main() {
  std::vector<int> arr;
  std::mutex mtx;
  std::thread t1([&] {
    for (int i = 0; i < 1000; i++) {
      std::lock_guard grd(mtx);
      arr.push_back(1);
    }
  });
  std::thread t2([&] {
    for (int i = 0; i < 1000; i++) {
      std::lock_guard grd(mtx);
      arr.push_back(2);
    }
  });
  t1.join();
  t2.join();
  return 0;
}
