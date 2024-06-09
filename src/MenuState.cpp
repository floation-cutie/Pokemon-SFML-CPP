#include "../include/MenuState.hpp"
#include "../include/Button.hpp"
#include "../include/HelperClass.hpp"
#include "../include/Label.hpp"
#include "../include/NetworkProtocol.hpp"
#include "../include/ResourceHolder.hpp"
#include "../include/Textfield.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <sys/socket.h>

unsigned int Game_Long_Connection_Port;

void MenuState::ConnectSocket() {
  // about socket
  struct sockaddr_in serv_addr;
  // 创建socket文件描述符
  if ((login_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cerr << "Socket creation error" << std::endl;
    exit(1);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(Server_Port);

  // 转换IP地址
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    std::cerr << "Invalid address or Address not supported" << std::endl;
    exit(1);
  }

  // 连接到服务器
  if (connect(login_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    std::cerr << "Connection failed" << std::endl;
    exit(1);
  }
}

MenuState::MenuState(StateStack &stack, Context context)
    : State(stack, context), mGUIContainer(), buf("") {
  sf::Texture &texture = context.textures->get(Textures::TitleScreen);
  mBackgroundSprite.setTexture(texture);

  auto UsernameInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  UsernameInputField->setPosition(50, 50);
  UsernameInputField->setText("Username");
  auto PassWordInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  PassWordInputField->setPosition(200, 50);
  PassWordInputField->setText("Password");

  auto hintLabel = std::make_shared<GUI::Label>("", *context.fonts);
  hintLabel->setPosition(100.f, 100.f);
  hintLabel->setCharSize(30);
  auto loginPlayButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  loginPlayButton->setPosition(100, 250);
  loginPlayButton->setText("Login");
  loginPlayButton->setCallback([=, this]() {
    ConnectSocket();
    buf = "login\n" + UsernameInputField->getText() + "\n" +
          PassWordInputField->getText() + "\n";
    if (send(login_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(login_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "Login Message sent" << std::endl;
    char recvBuf[BUF_LENGTH] = "";
    if (read(login_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(login_sock);
      exit(1);
    }
    int tmpPort = Helper::isValidPort(recvBuf);
    if (tmpPort >= 0) {
      Game_Long_Connection_Port = tmpPort;
      requestStackPop();
      requestStackPush(States::Game);
    } else {
      hintLabel->setText(std::string(recvBuf));
      hintLabel->setColor(sf::Color::Red);
    }
  });

  auto registerPlayButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  registerPlayButton->setPosition(100, 300);
  registerPlayButton->setText("Register");
  registerPlayButton->setCallback([=, this]() {
    ConnectSocket();
    std::cout << "UsernameInputField- " << UsernameInputField->getText()
              << std::endl;
    std::cout << "PassWordInputField-" << PassWordInputField->getText()
              << std::endl;
    buf = "register\n" + UsernameInputField->getText() + "\n" +
          PassWordInputField->getText() + "\n";
    if (send(login_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(login_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "Message sent" << std::endl;
    char recvBuf[BUF_LENGTH] = "";
    if (read(login_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(login_sock);
      exit(1);
    }
    //本界面的反馈消息
    hintLabel->setText(std::string(recvBuf));
    if (strcmp(recvBuf, "Accept.\n") == 0) {
      hintLabel->setColor(sf::Color::Green);
    } else {
      hintLabel->setColor(sf::Color::Red);
    }
  });
  auto settingsButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  settingsButton->setPosition(100, 350);
  settingsButton->setText("Settings");
  settingsButton->setCallback([this]() { requestStackPush(States::Settings); });

  auto exitButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  exitButton->setPosition(100, 400);
  exitButton->setText("Exit");
  exitButton->setCallback([this]() { requestStackPop(); });

  mGUIContainer.pack(UsernameInputField);
  mGUIContainer.pack(PassWordInputField);
  mGUIContainer.pack(hintLabel);
  mGUIContainer.pack(loginPlayButton);
  mGUIContainer.pack(registerPlayButton);
  mGUIContainer.pack(settingsButton);
  mGUIContainer.pack(exitButton);
}

void MenuState::draw() {
  sf::RenderWindow &window = *getContext().window;

  window.setView(window.getDefaultView());

  window.draw(mBackgroundSprite);
  window.draw(mGUIContainer);
}

bool MenuState::update(sf::Time) { return true; }

bool MenuState::handleEvent(const sf::Event &event) {
  mGUIContainer.handleEvent(event);
  return false;
}
