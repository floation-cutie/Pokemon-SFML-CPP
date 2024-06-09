#pragma once

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
#include <vector>
class ChooseBattleState : public State {
public:
  ChooseBattleState(StateStack &stack, Context context);

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event &event);

private:
  enum ChoosebattleLayer { Background, Spirits, LayerCount };
  TextureHolder mtextures;
  SceneNode mSceneGraph;
  GUI::Container mGUIContainer;
  std::array<SceneNode *, LayerCount> mLayers;
  std::string buf;
  components::spirits::Pokemon *ltPokemon, *rtPokemon, *ldPokemon, *rdPokemon;
  std::vector<int> pokemonids;
  void loadTextures();
  void buildScene();
};
