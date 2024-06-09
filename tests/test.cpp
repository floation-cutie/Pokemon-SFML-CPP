#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

int a = 4;
std::vector<std::string> split(const std::string &str, char delimiter = ' ') {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);

  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}
bool isValid(const string &str) {
  if (str[0] == '_')
    return false;
  for (auto c : str) {
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_')) {
      // not a letter or a digit or '_'
      return false;
    }
  }
  return true;
}
int ValidPort(const char *str) {
  try {
    return std::stoi(str);
  } catch (const std::invalid_argument &e) {
    // 如果转换失败，抛出 std::invalid_argument 异常
    return -1;
  }
}

int main() {
  struct in_addr addr;
  char ip_str[INET_ADDRSTRLEN]; // 定义一个足够大的字符串来存储IP地址

  // 设置网络字节序的IP地址
  addr.s_addr = inet_addr("192.168.1.1");

  // 使用inet_ntop函数进行转换
  if (inet_ntop(AF_INET, &addr, ip_str, INET_ADDRSTRLEN) == NULL) {
    perror("inet_ntop failed");
    return 1;
  }
  auto result = split("The result is\n", '\n');

  for (auto s : result) {
    cout << "The str is " << s << endl;
  }
  if (result.size() == 1) {
    cout << "TEST" << endl;
  }
  // 输出转换后的IP地址
  std::cout << "IP Address: " << ip_str << std::endl;
  std::cout << ValidPort("1e.23") << std::endl;
  std::cout << ValidPort("sfha") << std::endl;
  std::cout << ValidPort("122") << std::endl;
  std::cout << ValidPort("23.1") << std::endl;

  a = 4;

  std::cout << a << std::endl;

  if (isValid("123")) {
    std::cout << "Valid one\n";
  } else {
    std::cout << "inValid one\n";
  }
  return 0;
}
