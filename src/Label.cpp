#include "../include/Label.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace GUI {

Label::Label(const std::string &text, const FontHolder &fonts)
    : mText(text, fonts.get(Fonts::Main), 16) {}

bool Label::isSelectable() const { return false; }

void Label::handleEvent(const sf::Event &) {}

void Label::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  states.transform *= getTransform();
  target.draw(mText, states);
}

void Label::setText(const std::string &text) { mText.setString(text); }

void Label::setColor(sf::Color color) { mText.setFillColor(color); }

sf::Color Label::getColor() const { return mText.getFillColor(); }

void Label::setCharSize(unsigned int size) { mText.setCharacterSize(size); }
} // namespace GUI
