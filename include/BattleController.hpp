#pragma once

#include "NetworkProtocol.hpp"
#include "Pokemon.hpp"
#include <cstddef>
#include <memory>

class BattleController {
private:
  std::shared_ptr<components::spirits::Pokemon> p1, p2;
  bool autoFight;
  size_t timer1;
  size_t timer2;
  char recvBuf[BUF_LENGTH]; // BUF_LENGTH is a macro defined in
                            // NetworkProtocol.hpp, which is 1024
  int connectSock;
  std::string msg;

public:
  BattleController(std::shared_ptr<components::spirits::Pokemon> p1,
                   std::shared_ptr<components::spirits::Pokemon> p2,
                   bool autoFight, int connectSock);
  bool start();

private:
  void autoFightFunc();
  void manualFightFunc();
};
