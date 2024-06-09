#ifndef BOOK_RESOURCEIDENTIFIERS_HPP
#define BOOK_RESOURCEIDENTIFIERS_HPP

// Forward declaration of SFML classes
namespace sf {
class Texture;
class Font;
} // namespace sf

namespace Textures {
enum ID {
  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  Desert,
  GrassLand,
  tank,
  agile,
  defensive,
  strength,
  TitleScreen,
  BattleBackground,
  ButtonNormal,
  ButtonSelected,
  ButtonPressed,
  ButtonNormal_small,
  ButtonSelected_small,
  ButtonPressed_small,
  TextNormal,
  TextSelected
};
}
namespace Fonts {
enum ID { Main };
}

// Forward declaration and a few type definitions
template <typename Resource, typename Identifier> class ResourceHolder;

typedef ResourceHolder<sf::Texture, Textures::ID> TextureHolder;
typedef ResourceHolder<sf::Font, Fonts::ID> FontHolder;
#endif // BOOK_RESOURCEIDENTIFIERS_HPP
