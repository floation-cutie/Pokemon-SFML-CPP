#include "../include/HelperClass.hpp"
#include <sstream>

std::vector<std::string> Helper::split(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);

  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}
//由于网络端口不会是double等浮点类型,不需要过多判断
int Helper::isValidPort(const char *str) {
  try {
    // 使用 std::stoi 函数将字符数组转换为整数
    return std::stoi(str);
    // 如果转换成功，字符数组可以被解释为整数
  } catch (const std::invalid_argument &e) {
    // 如果转换失败，抛出 std::invalid_argument 异常
    return -1;
  }
}
