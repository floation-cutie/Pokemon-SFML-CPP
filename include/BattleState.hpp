#pragma once

#include "BattleController.hpp"
#include "Container.hpp"
#include "Pokemon.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"
#include "State.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

class BattleState : public State {
public:
  BattleState(StateStack &stack, Context context);

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event &event);

private:
  enum battleLayer { Background, Spirits, LayerCount };

  TextureHolder mtextures;
  sf::Text mBattleText;
  SceneNode mSceneGraph;
  GUI::Container mGUIContainer;
  std::array<SceneNode *, LayerCount> mLayers;
  char recvBuf[BUF_LENGTH];
  components::spirits::Pokemon *leftPlayerPokemon, *rightEnemyPokemon;
  BattleController *battle;
  bool inBattle;
  void loadTextures();
  void buildScene();
  void discard();
};
