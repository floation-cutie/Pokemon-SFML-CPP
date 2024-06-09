#include "../include/World.hpp"
//#include "../include/BattleController.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cmath>
#include <iostream>
#include <memory>
#include <thread>
// using std::cout;
// using std::endl;
World::World(sf::RenderWindow &window)
    : mWindow(window), mWorldView(window.getDefaultView()), mTextures(),
      mSceneGraph(), mSceneLayers(),
      mWorldBounds(0.f, 0.f, mWorldView.getSize().x, 2000.f),
      mSpawnPosition(mWorldView.getSize().x / 2.f,
                     mWorldBounds.height - mWorldView.getSize().y / 2.f),
      mGrassLandPosition(mWorldView.getSize().x / 8.f,
                         mWorldBounds.height -
                             3.f * mWorldView.getSize().y / 4.f),
      mScrollSpeed(0.f), mGamer(nullptr) {
  loadTextures();
  buildScene();

  // Prepare the view
  mWorldView.setCenter(mSpawnPosition);
}

void World::update(sf::Time dt) {
  // Scroll the world, reset player velocity
  mWorldView.move(0.f, mScrollSpeed * dt.asSeconds());
  mGamer->setVelocity(0.f, 0.f);
  // leftPlayerPokemon->setVelocity(0.f, 0.f);
  // rightPlayerPokemon->setVelocity(0.f, 0.f);
  //  Forward commands to scene graph, adapt velocity (scrolling, diagonal
  //  correction)
  while (!mCommandQueue.isEmpty()) {
    auto topCommand = mCommandQueue.pop();
    // 根据命令的类型，执行相应的操作
    switch (topCommand.category) {
    case Category::MoveDown:
      mGamer->setTexture(Gamer::Type::MoveDown, mTextures);
      break;
    case Category::MoveUp:
      mGamer->setTexture(Gamer::Type::MoveUp, mTextures);
      break;
    case Category::MoveLeft:
      mGamer->setTexture(Gamer::Type::MoveLeft, mTextures);
      break;
    case Category::MoveRight:
      mGamer->setTexture(Gamer::Type::MoveRight, mTextures);
      break;
    default:
      break;
    }
    mSceneGraph.onCommand(topCommand, dt);
  }
  adaptPlayerVelocity();
  // Regular update step, adapt position (correct if outside view)
  mSceneGraph.update(dt);
  adaptPlayerPosition();
}

void World::draw() {
  mWindow.setView(mWorldView);
  mWindow.draw(mSceneGraph);
}

CommandQueue &World::getCommandQueue() { return mCommandQueue; }

void World::loadTextures() {
  mTextures.load(Textures::MoveDown, "../source/Textures/MoveDown.png");
  mTextures.load(Textures::MoveUp, "../source/Textures/MoveUp.png");
  mTextures.load(Textures::MoveLeft, "../source/Textures/MoveLeft.png");
  mTextures.load(Textures::MoveRight, "../source/Textures/MoveRight.png");
  mTextures.load(Textures::Desert, "../source/Textures/Desert.png");
  mTextures.load(Textures::GrassLand, "../source/Textures/grassland.png");
}

void World::buildScene() {
  // Initialize the different layers
  for (std::size_t i = 0; i < LayerCount; ++i) {
    SceneNode::Ptr layer(new SceneNode());
    mSceneLayers[i] = layer.get();

    mSceneGraph.attachChild(std::move(layer));
  }

  // Prepare the tiled background
  sf::Texture &texture = mTextures.get(Textures::Desert);
  sf::IntRect textureRect(mWorldBounds);
  texture.setRepeated(true);

  // Add the background sprite to the scene
  std::unique_ptr<SpriteNode> backgroundSprite(
      new SpriteNode(texture, textureRect));
  backgroundSprite->setPosition(mWorldBounds.left, mWorldBounds.top);
  mSceneLayers[Background]->attachChild(std::move(backgroundSprite));

  // Add GrassLand sprite to the Scene
  sf::Texture &grassLandTexture = mTextures.get(Textures::GrassLand);
  sf::FloatRect grassLandBounds(0.f, 0.f, grassLandTexture.getSize().x,
                                grassLandTexture.getSize().y);
  sf::IntRect grassLandTextureRect(grassLandBounds);

  std::unique_ptr<SpriteNode> grassLandSprite(
      new SpriteNode(grassLandTexture, grassLandTextureRect));
  grassLandSprite->setPosition(mGrassLandPosition);
  mGrassLandBounds = grassLandSprite->getGlobalBounds();

  mSceneLayers[Background]->attachChild(std::move(grassLandSprite));
  // Add player
  std::unique_ptr<Gamer> leader(new Gamer(Gamer::MoveDown, mTextures));
  mGamer = leader.get();
  mGamer->setPosition(mSpawnPosition);
  mGamer->setVelocity(40.f, mScrollSpeed);
  // 此处将智能指针进行了转发，所以原指针的所有权被转移给了mSceneLayers[Air]
  mSceneLayers[Air]->attachChild(std::move(leader));
}

// 当玩家的位置接触到草坪的时候，就可以开始战斗
bool World::readyForBattle() const {
  //  std::this_thread::sleep_for(std::chrono::seconds(2));
  return mGamer->getGlobalBds().intersects(mGrassLandBounds);
}

void World::adaptPlayerPosition() {
  // Keep player's position inside the screen bounds, at least borderDistance
  // units from the border
  sf::FloatRect viewBounds(mWorldView.getCenter() - mWorldView.getSize() / 2.f,
                           mWorldView.getSize());
  const float borderDistance = 40.f;

  sf::Vector2f position = mGamer->getPosition();
  position.x = std::max(position.x, viewBounds.left + borderDistance);
  position.x =
      std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
  position.y = std::max(position.y, viewBounds.top + borderDistance);
  position.y =
      std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
  mGamer->setPosition(position);
  /*
    position = leftPlayerPokemon->getPosition();
    position.x = std::max(position.x, viewBounds.left + borderDistance);
    position.x =
        std::min(position.x, viewBounds.left + viewBounds.width -
    borderDistance); position.y = std::max(position.y, viewBounds.top +
    borderDistance); position.y = std::min(position.y, viewBounds.top +
    viewBounds.height - borderDistance);
    leftPlayerPokemon->setPosition(position);

    position = rightPlayerPokemon->getPosition();
    position.x = std::max(position.x, viewBounds.left + borderDistance);
    position.x =
        std::min(position.x, viewBounds.left + viewBounds.width -
    borderDistance); position.y = std::max(position.y, viewBounds.top +
    borderDistance); position.y = std::min(position.y, viewBounds.top +
    viewBounds.height - borderDistance);
    rightPlayerPokemon->setPosition(position);
    */
}

void World::adaptPlayerVelocity() {
  sf::Vector2f velocity = mGamer->getVelocity();
  mScrollSpeed = mGamer->getVelocity().y;
  // If moving diagonally, reduce velocity (to have always same velocity)
  if (velocity.x != 0.f && velocity.y != 0.f)
    mGamer->setVelocity(velocity / std::sqrt(2.f));

  mGamer->accelerate(0.f, mScrollSpeed / 2);
}
