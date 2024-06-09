#include "../include/GameServer.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
int main(int argc, char *argv[]) {
  srand(time(NULL));
  try {
    GameServer &server = GameServer::getInstance();
    server.start();
  } catch (std::exception e) {
    std::cout << e.what() << std::endl;
  }
  //中断服务器的方法
  return 0;
}
