#ifndef BOOK_CATEGORY_HPP
#define BOOK_CATEGORY_HPP

// Entity/scene node category, used to dispatch commands
namespace Category {
enum Type {
  None = 0,
  Scene = 1 << 0,
  MoveDown = 1 << 1,
  MoveUp = 1 << 2,
  MoveLeft = 1 << 3,
  MoveRight = 1 << 4,
};
}

#endif // BOOK_CATEGORY_HPP
