#include <iostream>
#include <string>
#include <vector>

#include <sstream>

std::vector<std::string> split(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);

  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

int main(int argc, char *argv[]) {
  std::string s = "dsa\n";
  auto v = split(s, '\n');
  std::cout << v[0];
  return 0;
}
