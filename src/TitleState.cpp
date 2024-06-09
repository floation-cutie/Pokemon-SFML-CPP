#include "../include/TitleState.hpp"
#include "../include/ResourceHolder.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <cmath>
TitleState::TitleState(StateStack &stack, Context context)
    : State(stack, context), mText(), mShowText(true),
      mTextEffectTime(sf::Time::Zero) {
  mBackgroundSprite.setTexture(context.textures->get(Textures::TitleScreen));

  mText.setFont(context.fonts->get(Fonts::Main));
  mText.setString("Press any key to start\nLogin in your own account please");
  mText.setFillColor(sf::Color::Cyan);
  sf::FloatRect bounds = mText.getLocalBounds();
  mText.setOrigin(std::floor(bounds.left + bounds.width / 2.f),
                  std::floor(bounds.top + bounds.height / 2.f));
  mText.setPosition(context.window->getView().getSize() / 2.f);
}

void TitleState::draw() {
  sf::RenderWindow &window = *getContext().window;
  window.draw(mBackgroundSprite);

  if (mShowText)
    window.draw(mText);
}

//实现闪烁功能
bool TitleState::update(sf::Time dt) {
  mTextEffectTime += dt;

  if (mTextEffectTime >= sf::seconds(0.5f)) {
    mShowText = !mShowText;
    mTextEffectTime = sf::Time::Zero;
  }

  return true;
}

bool TitleState::handleEvent(const sf::Event &event) {
  // If any key is pressed, trigger the next screen
  if (event.type == sf::Event::KeyPressed) {
    requestStackPop();
    requestStackPush(States::Menu);
  }

  return true;
}
