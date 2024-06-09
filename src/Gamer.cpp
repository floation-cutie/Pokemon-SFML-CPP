#include "../include/Gamer.hpp"
#include "../include/ResourceHolder.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
Textures::ID toTextureID(Gamer::Type type) {
  switch (type) {
  case Gamer::MoveLeft:
    return Textures::MoveLeft;
  case Gamer::MoveRight:
    return Textures::MoveRight;
  case Gamer::MoveUp:
    return Textures::MoveUp;
  case Gamer::MoveDown:
    return Textures::MoveDown;
  }
  return Textures::MoveDown;
}

Gamer::Gamer(Type type, const TextureHolder &textures)
    : mType(type), mSprite(textures.get(toTextureID(type))) {
  sf::FloatRect bounds = mSprite.getLocalBounds();
  mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

void Gamer::setTexture(Type type, const TextureHolder &textures) {
  mType = type;
  mSprite.setTexture(textures.get(toTextureID(type)));
  sf::FloatRect bounds = mSprite.getLocalBounds();
  mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

// 按位或运算符
unsigned int Gamer::getCategory() const {
  return Category::MoveDown | Category::MoveUp | Category::MoveLeft |
         Category::MoveRight;
}

sf::FloatRect Gamer::getGlobalBds() const {
  return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Gamer::drawCurrent(sf::RenderTarget &target,
                        sf::RenderStates states) const {
  target.draw(mSprite, states);
}
