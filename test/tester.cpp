#include "BattleController.hpp"
#include "Pokemon.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {

  auto leftPokemon = std::make_unique<components::spirits::Pokemon>(
      components::spirits::Pokemon::Species::strength, std::string("left"));
  //      std::shared_ptr<components::spirits::Pokemon>(leftPokemon.get());

  auto rightPokemon = std::make_unique<components::spirits::Pokemon>(
      components::spirits::Pokemon::Species::tank, std::string("right"));

  leftPokemon->upGrade(1000);
  rightPokemon->upGrade(1200);
  BattleController *battle = new BattleController(
      std::shared_ptr<components::spirits::Pokemon>(leftPokemon.get()),
      std::shared_ptr<components::spirits::Pokemon>(rightPokemon.get()));

  battle->start();
  std::cin.ignore(); // 等待用户输入
  return 0;
}
