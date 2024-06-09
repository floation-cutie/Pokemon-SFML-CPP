#ifndef BOOK_STATESTACK_HPP
#define BOOK_STATESTACK_HPP

#include "ResourceIdentifiers.hpp"
#include "State.hpp"
#include "StateIdentifiers.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>

#include <functional>
#include <map>
#include <utility>
#include <vector>

namespace sf {
class Event;
class RenderWindow;
} // namespace sf

class StateStack : private sf::NonCopyable {
public:
  enum Action {
    Push,
    Pop,
    Clear,
  };

public:
  explicit StateStack(State::Context context);

  template <typename T> void registerState(States::ID stateID);

  void update(sf::Time dt);
  void draw();
  void handleEvent(const sf::Event &event);

  void pushState(States::ID stateID);
  void popState();
  void clearStates();

  bool isEmpty() const;

private:
  State::Ptr createState(States::ID stateID);
  void applyPendingChanges();

private:
  struct PendingChange {
    explicit PendingChange(Action action, States::ID stateID = States::None);

    Action action;
    States::ID stateID;
  };

private:
  //状态
  std::vector<State::Ptr> mStack;
  //状态变化延迟
  std::vector<PendingChange> mPendingList;

  State::Context mContext;
  //构建实际对象的工厂函数
  std::map<States::ID, std::function<State::Ptr()>> mFactories;
};

template <typename T> void StateStack::registerState(States::ID stateID) {
  mFactories[stateID] = [this]() { return State::Ptr(new T(*this, mContext)); };
}

#endif // BOOK_STATESTACK_HPP
