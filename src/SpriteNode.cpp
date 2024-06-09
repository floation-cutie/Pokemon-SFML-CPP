#include "../include/SpriteNode.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

SpriteNode::SpriteNode(const sf::Texture &texture) : mSprite(texture) {}

SpriteNode::SpriteNode(const sf::Texture &texture, const sf::IntRect rect)
    : mSprite(texture, rect) {}

sf::FloatRect SpriteNode::getGlobalBounds() const {
  return getTransform().transformRect(mSprite.getGlobalBounds());
}
void SpriteNode::drawCurrent(sf::RenderTarget &target,
                             sf::RenderStates states) const {
  target.draw(mSprite, states);
}
