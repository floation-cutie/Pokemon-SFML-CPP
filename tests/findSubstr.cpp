#include <iostream>
#include <string>

int main() {
  std::string str = "Hello, 阿宸!";
  std::string substring = "阿宸";

  // 使用 std::string::npos 判断是否包含子字符串
  if (str.find(substring) != std::string::npos) {
    std::cout << "Found the substring!" << std::endl;
  } else {
    std::cout << "Substring not found." << std::endl;
  }

  return 0;
}
