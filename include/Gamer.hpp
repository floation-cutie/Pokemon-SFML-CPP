#pragma once
#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>

class Gamer : public Entity {
public:
  enum Type {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
  };

public:
  Gamer(Type type, const TextureHolder &textures);
  void setTexture(Type type, const TextureHolder &textures);
  sf::FloatRect getGlobalBds() const;
  virtual unsigned int getCategory() const;

private:
  virtual void drawCurrent(sf::RenderTarget &target,
                           sf::RenderStates states) const;

private:
  Type mType;
  sf::Sprite mSprite;
};
