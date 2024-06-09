#include "../include/Textfield.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <iostream>
#include <ostream>
#include <regex>
#include <sstream>
namespace GUI {

Textfield::Textfield(const FontHolder &fonts, const TextureHolder &textures)
    : mText("", fonts.get(Fonts::Main), 16),
      mNormalTexture(textures.get(Textures::TextNormal)),
      mSelectedTexture(textures.get(Textures::TextSelected)), ss(""),
      mSprite() {
  mSprite.setTexture(mNormalTexture);

  sf::FloatRect bounds = mSprite.getLocalBounds();
  mText.setPosition(bounds.width / 2.f, bounds.height / 2.f);
}

bool Textfield::isSelectable() const { return true; }
void Textfield::select() {
  Component::select();

  mSprite.setTexture(mSelectedTexture);
}

void Textfield::deselect() {
  Component::deselect();

  mSprite.setTexture(mNormalTexture);
}
void Textfield::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Escape && isActive()) {
      deactivate();
    } else if (event.key.code == sf::Keyboard::Backspace) {
      // 将流内容转换为字符串
      std::string str = ss.str();
      // 删除字符串末尾的字符
      if (!str.empty()) {
        str.pop_back(); // 删除最后一个字符
      }
      // 将修改后的字符串重新赋值给流
      ss.str(str);
    }

    setText(ss.str());
  } else if (event.type == sf::Event::TextEntered && event.key.code != 8 &&
             event.key.code != 13) {
    // std::cout << event.key.code << std::endl;
    ss.seekp(0, std::ios::end);
    ss << static_cast<char>(event.text.unicode);
    setText(ss.str());
  }
}

void Textfield::activate() { Component::activate(); }

void Textfield::deactivate() { Component::deactivate(); }

void Textfield::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  states.transform *= getTransform();
  target.draw(mSprite, states);
  target.draw(mText, states);
}

void Textfield::setText(const std::string &text) {
  mText.setString(text);
  sf::FloatRect bounds = mSprite.getLocalBounds();
  mText.setOrigin(std::floor(bounds.left + bounds.width / 2.f),
                  std::floor(bounds.top + bounds.height / 2.f));
}

std::string Textfield::getText() const { return ss.str(); }

} // namespace GUI
