#pragma once
#include "Component.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <sstream>
namespace GUI {

class Textfield : public Component {
public:
  typedef std::shared_ptr<Textfield> Ptr;

public:
  Textfield(const FontHolder &fonts, const TextureHolder &textures);

  virtual bool isSelectable() const;
  virtual void select();
  virtual void deselect();
  virtual void activate();
  virtual void deactivate();
  void setText(const std::string &text);
  std::string getText() const;
  virtual void handleEvent(const sf::Event &event);

private:
  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

private:
  sf::Text mText;
  const sf::Texture &mNormalTexture;
  const sf::Texture &mSelectedTexture;
  sf::Sprite mSprite;
  std::stringstream ss;
};
} // namespace GUI
