#ifndef BOOK_GAMESTATE_HPP
#define BOOK_GAMESTATE_HPP

#include "Player.hpp"
#include "State.hpp" //由于有StateStack的前向声明,可以在此不引入StateStack
#include "World.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

class GameState : public State {
public:
  GameState(StateStack &stack, Context context);

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event &event);
  void ConnectSocket();

private:
  World mWorld;
  Player &mPlayer;
  bool fight = false;
};

#endif // BOOK_GAMESTATE_HPP
