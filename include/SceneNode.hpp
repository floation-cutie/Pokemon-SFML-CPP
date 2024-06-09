/*

The scene graph owns the scene nodes, therefore it is responsible for their
lifetime and destruction. Therefore, each node stores all of its child nodes,
which get destroyed if the node is destroyed.

SceneNode inherits from:
- sf::Drawable, since a scene node is meant to be rendered; the purely virtual
draw() function of sf::Drawable is overriden;
- sf::Transformable, since it needs to store its current position, rotation and
scale;
- sf::NonCopyable (privately), since a scene node cannot be copied.

*/

#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include "Command.hpp"
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>
#include <memory>
#include <vector>

class SceneNode : public sf::Transformable,
                  public sf::Drawable,
                  private sf::NonCopyable {
public:
  typedef std::unique_ptr<SceneNode> Ptr;

public:
  SceneNode();

  void attachChild(Ptr child);
  Ptr detachChild(const SceneNode &node);

  void update(sf::Time dt);

  sf::Vector2f getWorldPosition() const;
  sf::Transform getWorldTransform() const;

  void onCommand(const Command &command, sf::Time dt);
  virtual unsigned int getCategory() const;

private:
  virtual void updateCurrent(sf::Time dt);
  void updateChildren(sf::Time dt);

  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;
  virtual void drawCurrent(sf::RenderTarget &target,
                           sf::RenderStates states) const;
  void drawChildren(sf::RenderTarget &target, sf::RenderStates states) const;

private:
  std::vector<Ptr> mChildren;
  SceneNode *mParent;
};

#endif // SCENENODE_HPP
