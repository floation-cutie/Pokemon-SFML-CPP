#ifndef BOOK_MENUSTATE_HPP
#define BOOK_MENUSTATE_HPP

#include "Container.hpp"
#include "State.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class MenuState : public State {
public:
  MenuState(StateStack &stack, Context context);

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event &event);
  void ConnectSocket();

private:
  sf::Sprite mBackgroundSprite;
  GUI::Container mGUIContainer;
  int login_sock;
  std::string buf; //对于客户端使用字符串类，方便操作
};

#endif // BOOK_MENUSTATE_HPP
