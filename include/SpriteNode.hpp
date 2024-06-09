/*

The SpriteNode class inherits SceneNode and provides a convenient type of node
for background sprites, which are not entities. It only contains one sprite with
one associated texture.

*/

#ifndef SPRITENODE_HPP
#define SPRITENODE_HPP

#include "SceneNode.hpp"

#include <SFML/Graphics/Sprite.hpp>

class SpriteNode : public SceneNode {
public:
  explicit SpriteNode(const sf::Texture &texture);
  SpriteNode(const sf::Texture &texture, const sf::IntRect rect);
  sf::FloatRect getGlobalBounds() const;

private:
  virtual void drawCurrent(sf::RenderTarget &target,
                           sf::RenderStates states) const;

private:
  sf::Sprite mSprite;
};

#endif // SPRITENODE_HPP
