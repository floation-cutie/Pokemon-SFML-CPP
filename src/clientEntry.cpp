#include "../include/Application.hpp"

#include <exception>
#include <iostream>

int main() {
  try {
    Application game;
    game.run();
  } catch (std::exception &e) {
    std::cout << "\nEXCEPTION: " << e.what() << std::endl;
  }
}
