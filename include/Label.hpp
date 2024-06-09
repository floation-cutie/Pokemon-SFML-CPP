#ifndef BOOK_LABEL_HPP
#define BOOK_LABEL_HPP

#include "Component.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

namespace GUI {

class Label : public Component {
public:
  typedef std::shared_ptr<Label> Ptr;

public:
  Label(const std::string &text, const FontHolder &fonts);

  virtual bool isSelectable() const;
  void setText(const std::string &text);
  void setColor(sf::Color color);
  void setCharSize(unsigned int size);
  virtual void handleEvent(const sf::Event &event);
  sf::Color getColor() const;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const;

private:
  sf::Text mText;
};

} // namespace GUI

#endif // BOOK_LABEL_HPP
