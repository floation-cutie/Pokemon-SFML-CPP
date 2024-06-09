#include <iostream>
#include <string>
#include <thread>

void download(std::string file) {
  for (int i = 0; i < 10; i++) {
    std::cout << "Downloading " << file << " (" << i * 10 << "%)..."
              << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
  }
  std::cout << "Download complete: " << file << std::endl;
}

void interact() {
  std::string name;
  std::cin >> name;
  std::cout << "Hi, " << name << std::endl;
}

//主线程退出 会导致子线程从属退出
int main() {
  std::thread t1([&] { download("hello.zip"); });
  interact();
  std::cout << "Waiting for child thread..." << std::endl;
  t1.join();
  std::cout << "Child thread exited!" << std::endl;
  return 0;
}
