#pragma once

#include "Pokemon.hpp"
#include <cstddef>
#include <memory>

class BattleController {
private:
  std::shared_ptr<components::spirits::Pokemon> p1, p2;
  bool autoFight;
  size_t timer1;
  size_t timer2;

public:
  BattleController(std::shared_ptr<components::spirits::Pokemon> p1,
                   std::shared_ptr<components::spirits::Pokemon> p2,
                   bool autoFight = true);
  void start();
};
