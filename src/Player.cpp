#include "../include/Player.hpp"
#include "../include/Command.hpp"
#include "../include/CommandQueue.hpp"
#include "../include/Gamer.hpp"
#include <algorithm>
#include <map>
#include <string>

struct GamerMover {
  GamerMover(float vx, float vy) : velocity(vx, vy) {}

  void operator()(Gamer &aircraft, sf::Time) const {
    aircraft.accelerate(velocity);
  }

  sf::Vector2f velocity;
};

Player::Player() {
  // Set initial key bindings
  mKeyBinding[sf::Keyboard::Left] = MoveLeft;
  mKeyBinding[sf::Keyboard::Right] = MoveRight;
  mKeyBinding[sf::Keyboard::Up] = MoveUp;
  mKeyBinding[sf::Keyboard::Down] = MoveDown;

  // Set initial action bindings
  initializeActions();
}

void Player::handleEvent(const sf::Event &event, CommandQueue &commands) {
  if (event.type == sf::Event::KeyPressed) {
    // Check if pressed key appears in key binding, trigger command if so
    auto found = mKeyBinding.find(event.key.code);
    if (found != mKeyBinding.end() && !isRealtimeAction(found->second))
      commands.push(mActionBinding[found->second]);
  }
}

void Player::handleRealtimeInput(CommandQueue &commands) {
  // Traverse all assigned keys and check if they are pressed
  for (auto pair : mKeyBinding) {
    // If key is pressed, lookup action and trigger corresponding command
    if (sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
      commands.push(mActionBinding[pair.second]);
  }
}

void Player::assignKey(Action action, sf::Keyboard::Key key) {
  // Remove all keys that already map to action
  for (auto itr = mKeyBinding.begin(); itr != mKeyBinding.end();) {
    if (itr->second == action)
      mKeyBinding.erase(itr++);
    else
      ++itr;
  }

  // Insert new binding
  mKeyBinding[key] = action;
}

sf::Keyboard::Key Player::getAssignedKey(Action action) const {
  for (auto pair : mKeyBinding) {
    if (pair.second == action)
      return pair.first;
  }

  return sf::Keyboard::Unknown;
}

void Player::initializeActions() {
  const float playerSpeed = 200.f;

  mActionBinding[MoveLeft].action =
      derivedAction<Gamer>(GamerMover(-playerSpeed, 0.f));
  mActionBinding[MoveLeft].category = Category::MoveLeft;
  mActionBinding[MoveRight].action =
      derivedAction<Gamer>(GamerMover(+playerSpeed, 0.f));
  mActionBinding[MoveRight].category = Category::MoveRight;
  mActionBinding[MoveUp].action =
      derivedAction<Gamer>(GamerMover(0.f, -playerSpeed));
  mActionBinding[MoveUp].category = Category::MoveUp;
  mActionBinding[MoveDown].action =
      derivedAction<Gamer>(GamerMover(0.f, +playerSpeed));
  mActionBinding[MoveDown].category = Category::MoveDown;
}

bool Player::isRealtimeAction(Action action) {
  switch (action) {
  case MoveLeft:
  case MoveRight:
  case MoveDown:
  case MoveUp:
    return true;

  default:
    return false;
  }
}
