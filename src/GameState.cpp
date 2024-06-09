#include "../include/GameState.hpp"
#include "../include/Button.hpp"
#include "../include/HelperClass.hpp"
#include "../include/NetworkProtocol.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
extern unsigned int Game_Long_Connection_Port;
int connect_sock;

void GameState::ConnectSocket() {
  // about socket
  struct sockaddr_in serv_addr;
  // 创建socket文件描述符
  if ((connect_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cerr << "Socket creation error" << std::endl;
    exit(1);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(Game_Long_Connection_Port);

  // 转换IP地址
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    std::cerr << "Invalid address or Address not supported" << std::endl;
    exit(1);
  }

  // 连接到服务器
  if (connect(connect_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    std::cerr << "Connection failed" << std::endl;
    exit(1);
  }
}
//显式表示
GameState::GameState(StateStack &stack, Context context)
    : State(stack, context), mWorld(World(*context.window)),
      mPlayer(*context.player) {
  ConnectSocket();
}

void GameState::draw() { mWorld.draw(); }

bool GameState::update(sf::Time dt) {
  mWorld.update(dt);
  if (!mWorld.readyForBattle()) {
    fight = false;
  } else if (!fight) {
    fight = true;
    requestStackPush(States::ChooseBattle);
  }
  CommandQueue &commands = mWorld.getCommandQueue();
  mPlayer.handleRealtimeInput(commands);

  return true;
}

bool GameState::handleEvent(const sf::Event &event) {
  // Game input handling
  CommandQueue &commands = mWorld.getCommandQueue();
  mPlayer.handleEvent(event, commands);

  // Escape pressed, trigger the pause screen
  if (event.type == sf::Event::KeyPressed &&
      event.key.code == sf::Keyboard::Escape)
    requestStackPush(States::Pause);

  return true;
}
