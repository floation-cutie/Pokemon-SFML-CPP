#include "../include/SmallButton.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <cmath>

namespace GUI {

SmallButton::SmallButton(const FontHolder &fonts, const TextureHolder &textures)
    : mCallback(), mNormalTexture(textures.get(Textures::ButtonNormal_small)),
      mSelectedTexture(textures.get(Textures::ButtonSelected_small)),
      mPressedTexture(textures.get(Textures::ButtonPressed_small)), mSprite(),
      mText("", fonts.get(Fonts::Main), 16), mIsToggle(false) {
  mSprite.setTexture(mNormalTexture);

  sf::FloatRect bounds = mSprite.getLocalBounds();
  mText.setPosition(bounds.width / 2.f, bounds.height / 2.f);
}

void SmallButton::setCallback(Callback callback) {
  mCallback = std::move(callback);
}

void SmallButton::setText(const std::string &text) {
  mText.setString(text);
  sf::FloatRect bounds = mText.getLocalBounds();
  mText.setOrigin(std::floor(bounds.left + bounds.width / 2.f),
                  std::floor(bounds.top + bounds.height / 2.f));
}

std::string SmallButton::getText() const { return mText.getString(); }

void SmallButton::setToggle(bool flag) { mIsToggle = flag; }

bool SmallButton::isSelectable() const { return true; }

void SmallButton::select() {
  Component::select();

  mSprite.setTexture(mSelectedTexture);
}

void SmallButton::deselect() {
  Component::deselect();

  mSprite.setTexture(mNormalTexture);
}

void SmallButton::activate() {
  Component::activate();

  // If we are toggle then we should show that the button is pressed and thus
  // "toggled".
  if (mIsToggle)
    mSprite.setTexture(mPressedTexture);

  if (mCallback)
    mCallback();

  // If we are not a toggle then deactivate the button since we are just
  // momentarily activated.
  if (!mIsToggle)
    deactivate();
}

void SmallButton::deactivate() {
  Component::deactivate();

  if (mIsToggle) {
    // Reset texture to right one depending on if we are selected or not.
    if (isSelected())
      mSprite.setTexture(mSelectedTexture);
    else
      mSprite.setTexture(mNormalTexture);
  }
}

void SmallButton::handleEvent(const sf::Event &) {}

void SmallButton::draw(sf::RenderTarget &target,
                       sf::RenderStates states) const {
  states.transform *= getTransform();
  target.draw(mSprite, states);
  target.draw(mText, states);
}

} // namespace GUI
