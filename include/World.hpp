#ifndef BOOK_WORLD_HPP
#define BOOK_WORLD_HPP

#include "Command.hpp"
#include "CommandQueue.hpp"
#include "Gamer.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/NonCopyable.hpp>

#include <array>
#include <memory>

// Forward declaration
namespace sf {
class RenderWindow;
}

class World : private sf::NonCopyable {
public:
  explicit World(sf::RenderWindow &window);
  void update(sf::Time dt);
  void draw();
  CommandQueue &getCommandQueue();
  bool readyForBattle() const;

private:
  void loadTextures();
  void buildScene();
  void adaptPlayerPosition();
  void adaptPlayerVelocity();

private:
  enum Layer { Background, Air, LayerCount };

private:
  sf::RenderWindow &mWindow;
  sf::View mWorldView;
  TextureHolder mTextures;
  SceneNode mSceneGraph;
  std::array<SceneNode *, LayerCount> mSceneLayers;
  CommandQueue mCommandQueue;
  sf::FloatRect mGrassLandBounds;
  sf::FloatRect mWorldBounds;
  sf::Vector2f mSpawnPosition;
  sf::Vector2f mGrassLandPosition;
  float mScrollSpeed;
  Gamer *mGamer;
};

#endif // BOOK_WORLD_HPP
