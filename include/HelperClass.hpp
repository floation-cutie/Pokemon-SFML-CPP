#pragma once
#include <string>
#include <vector>
// Helper函数，处理网络中的字符串

class Helper {
public:
  static std::vector<std::string> split(const std::string &str,
                                        char delimiter = ' ');
  static int isValidPort(const char *str);
};
