#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
using namespace std;

int main(int argc, char *argv[]) {
  auto t0 = chrono::steady_clock::now();
  auto t1 = t0 + chrono::seconds(40);
  auto dt = t0 - t1;
  int64_t sec = chrono::duration_cast<chrono::seconds>(dt).count();
  cout << "time elapsed: " << sec << "s" << endl;

  //跨平台统一API
  std::this_thread::sleep_for(chrono::milliseconds(400));
  return 0;
}
