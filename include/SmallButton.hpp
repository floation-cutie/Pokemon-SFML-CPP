
#ifndef BOOK_BUTTON_HPP
#define BOOK_BUTTON_HPP

#include "Component.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace GUI {

class SmallButton : public Component {
public:
  typedef std::shared_ptr<SmallButton> Ptr;
  typedef std::function<void()> Callback;

public:
  SmallButton(const FontHolder &fonts, const TextureHolder &textures);

  void setCallback(Callback callback);
  void setText(const std::string &text);
  void setToggle(bool flag);
  std::string getText() const;
  virtual bool isSelectable() const;
  virtual void select();
  virtual void deselect();

  virtual void activate();
  virtual void deactivate();

  virtual void handleEvent(const sf::Event &event);

private:
  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

private:
  Callback mCallback;
  const sf::Texture &mNormalTexture;
  const sf::Texture &mSelectedTexture;
  const sf::Texture &mPressedTexture;
  sf::Sprite mSprite;
  sf::Text mText;
  bool mIsToggle;
};

} // namespace GUI

#endif // BOOK_BUTTON_HPP
