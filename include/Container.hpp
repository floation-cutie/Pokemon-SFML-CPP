#ifndef BOOK_CONTAINER_HPP
#define BOOK_CONTAINER_HPP

#include "Component.hpp"

#include <memory>
#include <vector>

namespace GUI {

class Container : public Component {
public:
  typedef std::shared_ptr<Container> Ptr;

public:
  Container();
  //将组件进行封装
  void pack(Component::Ptr component);

  virtual bool isSelectable() const;
  virtual void handleEvent(const sf::Event &event);

private:
  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

  bool hasSelection() const;
  void select(std::size_t index);
  void selectNext();
  void selectPrevious();

private:
  std::vector<Component::Ptr> mChildren;
  // 每组容器中有且只有一个被选中的组件
  int mSelectedChild;
};

} // namespace GUI

#endif // BOOK_CONTAINER_HPP
