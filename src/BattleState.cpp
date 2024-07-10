#include "../include/BattleState.hpp"
#include "../include/HelperClass.hpp"
#include "../include/Label.hpp"
#include "../include/NetworkProtocol.hpp"
#include "../include/ResourceHolder.hpp"
#include "../include/SmallButton.hpp"
#include "../include/Textfield.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Context.hpp>
#include <arpa/inet.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

extern int connect_sock;

// 仿照PauseState的构造函数，实现BattleState的构造函数
BattleState::BattleState(StateStack &stack, Context context)
    : State(stack, context), mtextures(), mGUIContainer(),
      leftPlayerPokemon(nullptr), rightEnemyPokemon(nullptr), battle(nullptr),
      inBattle(true), inBattleState(true) {
  loadTextures();
  buildScene();
}

void BattleState::loadTextures() {
  mtextures.load(Textures::BattleBackground,
                 "../source/Textures/battlebgForest.png");
  mtextures.load(Textures::defensive, "../source/Textures/defensive_small.png");
  mtextures.load(Textures::agile, "../source/Textures/agile_small.png");
  mtextures.load(Textures::strength, "../source/Textures/strength_small.png");
  mtextures.load(Textures::tank, "../source/Textures/tank_small.png");
}

void BattleState::discard() {
  // 呈现一个提供输入的输入框，让用户输入要丢弃的精灵的id
  // 和一个确认按钮
  sf::Vector2u windowSize(getContext().window->getSize());
  auto discardLabel = std::make_shared<GUI::Label>("", *getContext().fonts);
  discardLabel->setPosition(windowSize.x / 2.f - 50,
                            3 * windowSize.y / 4.f - 50);
  discardLabel->setText("Discard");
  discardLabel->setCharSize(30);

  auto discardInputField = std::make_shared<GUI::Textfield>(
      *getContext().fonts, *getContext().textures);
  discardInputField->setPosition(windowSize.x / 2.f - 50,
                                 3 * windowSize.y / 4.f);
  discardInputField->setText("");

  auto discardButton = std::make_shared<GUI::SmallButton>(
      *getContext().fonts, *getContext().textures);
  discardButton->setPosition(windowSize.x / 2.f - 50,
                             3 * windowSize.y / 4.f + 50);
  discardButton->setText("Ready to discard");
  discardButton->setCallback([=, this]() {
    int discardId = Helper::isValidPort(discardInputField->getText().c_str());
    if (discardId == -1) {
      discardLabel->setColor(sf::Color::Red);
      discardLabel->setText("Invalid id!");
      return;
    } else {
      std::string buf = "discard\n" + discardInputField->getText() + "\n";
      if (send(connect_sock, buf.c_str(), BUF_LENGTH, 0) < 0) {
        std::cerr << "Error writing to server." << std::endl;
        close(connect_sock); // 正确关闭socket
        exit(1);
      }
      memset(recvBuf, 0, BUF_LENGTH);
      if (recv(connect_sock, recvBuf, BUF_LENGTH, 0) < 0) {
        std::cerr << "Error receive message from server" << std::endl;
        close(connect_sock);
        exit(1);
      }
      auto strs = Helper::split(std::string(recvBuf), '\n');
      if (strs[0] == "Accept.") {
        discardLabel->setColor(sf::Color::Green);
        discardLabel->setText("Discard successfully!");
        inBattleState = false;
      } else {
        discardLabel->setColor(sf::Color::Red);
        discardLabel->setText("Discard failed!");
      }
    }
  });

  mGUIContainer.pack(discardLabel);
  mGUIContainer.pack(discardInputField);
  mGUIContainer.pack(discardButton);
}

void BattleState::buildScene() {
  // Initialize the different layers
  for (std::size_t i = 0; i < LayerCount; ++i) {
    SceneNode::Ptr layer(new SceneNode());
    mLayers[i] = layer.get();

    mSceneGraph.attachChild(std::move(layer));
  }

  // 将背景图片设置为战斗背景,大小覆盖整个窗口
  sf::Texture &texture = mtextures.get(Textures::BattleBackground);
  // 从context中获取窗口大小
  sf::Vector2u windowSize(getContext().window->getSize());
  sf::IntRect textureRect(0, 0, 500 + windowSize.x, 500 + windowSize.y);
  texture.setRepeated(true);

  std::unique_ptr<SpriteNode> backgroundSprite(
      new SpriteNode(texture, textureRect));
  backgroundSprite->setPosition(0, 0);
  mLayers[Background]->attachChild(std::move(backgroundSprite));
  // 左右各放置一个精灵
  // 从服务器中获取精灵信息,为两者的种类和名字
  if (recv(connect_sock, recvBuf, BUF_LENGTH, 0) < 0) {
    std::cerr << "Error receive message from server" << std::endl;
    close(connect_sock);
    exit(1);
  }
  auto strs = Helper::split(std::string(recvBuf), '\n');
  auto leftPokemon = std::make_unique<components::spirits::Pokemon>(
      components::spirits::Pokemon::Species(std::stoi(strs[0])), strs[1],
      mtextures);
  leftPlayerPokemon = leftPokemon.get();
  leftPlayerPokemon->setPosition(windowSize.x / 4.f, windowSize.y / 2.f);
  mLayers[Spirits]->attachChild(std::move(leftPokemon));

  auto rightPokemon = std::make_unique<components::spirits::Pokemon>(
      components::spirits::Pokemon::Species(std::stoi(strs[2])), strs[3],
      mtextures);
  rightEnemyPokemon = rightPokemon.get();
  rightEnemyPokemon->setPosition(3.f * windowSize.x / 4.f, windowSize.y / 2.f);
  mLayers[Spirits]->attachChild(std::move(rightPokemon));
  // 服务器此时已经进入战斗状态
  // 提示信息
  auto hintLabel = std::make_shared<GUI::Label>("", *getContext().fonts);
  hintLabel->setPosition(50, 100);
  hintLabel->setText("Battle");
  hintLabel->setCharSize(30);

  // 普通攻击按钮
  auto attackButton = std::make_shared<GUI::SmallButton>(
      *getContext().fonts, *getContext().textures);
  attackButton->setPosition(530, 250);
  attackButton->setText(leftPlayerPokemon->GetSkillName(0));
  attackButton->setCallback([=, this]() mutable {
    if (!inBattle)
      return;
    // 发送攻击请求
    // 格式为 action \n skillIndex \n
    std::string buf = "attack\n0\n";
    if (send(connect_sock, buf.c_str(), BUF_LENGTH, 0) < 0) {
      hintLabel->setColor(sf::Color::Red);
      hintLabel->setText("Error writing to server.");
      close(connect_sock); // 正确关闭socket
    }
    std::cout << "Normal Attack Message sent" << std::endl;
    memset(recvBuf, 0, BUF_LENGTH);
    if (recv(connect_sock, recvBuf, BUF_LENGTH, 0) < 0) {
      hintLabel->setColor(sf::Color::Red);
      hintLabel->setText("Error receive message from server");
      close(connect_sock);
    }
    // 本界面的反馈消息
    std::cout << "recvBuf: \n" << recvBuf << std::endl;

    auto splits = Helper::split(std::string(recvBuf), '\n');
    if (splits.size() == 1 && splits[0] == "Win") {
      hintLabel->setColor(sf::Color::Green);
      hintLabel->setText("You win!");
      inBattle = false;
      inBattleState = false;
    } else if (splits.size() == 1 && splits[0] == "Lose") {
      hintLabel->setColor(sf::Color::Red);
      hintLabel->setText("You lose!");
      inBattle = false;
      buf = "Lose response\n";
      if (send(connect_sock, buf.c_str(), BUF_LENGTH, 0) < 0) {
        std::cerr << "Error writing to server." << std::endl;
        close(connect_sock); // 正确关闭socket
        exit(1);
      }
      //持续监听 服务器的消息 直到收到Duel 或者 Upgrade
      while (true) {
        memset(recvBuf, 0, BUF_LENGTH);
        if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
          std::cerr << "Error receive message from server" << std::endl;
          close(connect_sock);
          exit(1);
        }
        auto strs = Helper::split(std::string(recvBuf), '\n');
        if (strs[0] == "Duel") {
          hintLabel->setColor(sf::Color::Red);
          std::string text =
              "You lose!\nNeed to throw a pokemon to continue.\n The ids are";
          for (int i = 1; i < strs.size(); i++) {
            text = text + " " + strs[i];
          }
          hintLabel->setText(text);
          discard();
          break;
        } else if (strs[0] == "Upgrade") {
          // do nothing
          hintLabel->setColor(sf::Color::Red);
          hintLabel->setText("You lose!");
          inBattleState = false;
          break;
        }
      }
    } else {
      std::string lastSentence = splits[splits.size() - 1];
      std::string normalString = "normal attack";
      if (lastSentence.find(normalString) != std::string::npos) {
        hintLabel->setColor(sf::Color::Green);
      } else {
        hintLabel->setColor(sf::Color::Red);
      }
      hintLabel->setText(lastSentence);
    }
  });

  // 技能攻击按钮
  auto skillButton = std::make_shared<GUI::SmallButton>(*getContext().fonts,
                                                        *getContext().textures);
  skillButton->setPosition(530, 300);
  skillButton->setText(leftPlayerPokemon->GetSkillName(1));
  skillButton->setCallback([=, this]() {
    if (!inBattle)
      return;
    // 发送攻击请求
    // 格式为 action \n skillIndex \n
    std::string buf = "attack\n1\n";
    if (send(connect_sock, buf.c_str(), BUF_LENGTH, 0) < 0) {
      hintLabel->setColor(sf::Color::Red);
      hintLabel->setText("Error writing to server.");
      close(connect_sock); // 正确关闭socket
    }
    std::cout << "Skill Attack Message sent" << std::endl;
    memset(recvBuf, 0, BUF_LENGTH);
    if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
      hintLabel->setColor(sf::Color::Red);
      hintLabel->setText("Error receive message from server");
      close(connect_sock);
    }
    // 本界面的反馈消息
    std::cout << "recvBuf: \n" << recvBuf << std::endl;
    auto splits = Helper::split(std::string(recvBuf), '\n');
    if (splits.size() == 1 && splits[0] == "Win") {
      hintLabel->setColor(sf::Color::Green);
      hintLabel->setText("You win!");
      inBattle = false;
      inBattleState = false;
    } else if (splits.size() == 1 && splits[0] == "Lose") {
      hintLabel->setColor(sf::Color::Red);
      hintLabel->setText("You lose!");
      inBattle = false;
      // 给服务器响应

      buf = "Lose response\n";
      if (send(connect_sock, buf.c_str(), BUF_LENGTH, 0) < 0) {
        std::cerr << "Error writing to server." << std::endl;
        close(connect_sock); // 正确关闭socket
        exit(1);
      }
      memset(recvBuf, 0, BUF_LENGTH);
      if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
        std::cerr << "Error receive message from server" << std::endl;
        close(connect_sock);
        exit(1);
      }
      auto strs = Helper::split(std::string(recvBuf), '\n');
      if (strs[0] == "Duel") {
        hintLabel->setColor(sf::Color::Red);
        std::string text = "You lose! Need to throw a pokemon to continue.\n";
        for (int i = 1; i < strs.size(); i++) {
          text = text + " " + strs[i];
        }
        hintLabel->setText(text);
        discard();
      } else if (strs[0] == "Upgrade") {
        // do nothing
        hintLabel->setColor(sf::Color::Red);
        hintLabel->setText("You lose!");
        inBattleState = false;
      }
    } else {
      std::string lastSentence = splits[splits.size() - 1];
      std::string normalString = "normal attack";
      if (lastSentence.find(normalString) != std::string::npos) {
        hintLabel->setColor(sf::Color::Green);
      } else {
        hintLabel->setColor(sf::Color::Red);
      }
      hintLabel->setText(lastSentence);
    }
  });

  // 返回按钮
  auto backButton = std::make_shared<GUI::SmallButton>(*getContext().fonts,
                                                       *getContext().textures);
  backButton->setPosition(530, 350);
  backButton->setText("Back");
  memset(recvBuf, 0, BUF_LENGTH);

  // 当战斗结束时，返回上一个界面
  backButton->setCallback([=, this]() {
    if (!inBattle && hintLabel->getColor() == sf::Color::Green)
      requestStackPop();
    else if (!inBattleState && hintLabel->getColor() == sf::Color::Red) {
      requestStackPop();
      // requestStackPush(GameOver);
    } else {
      hintLabel->setText("Battle is not over yet!");
    }
  });

  // 将各个控件添加到mGUIContainer中
  mGUIContainer.pack(hintLabel);
  mGUIContainer.pack(attackButton);
  mGUIContainer.pack(skillButton);
  mGUIContainer.pack(backButton);
}

void BattleState::draw() {
  sf::RenderWindow &window = *getContext().window;
  window.setView(window.getDefaultView());

  window.draw(mSceneGraph);
  window.draw(mGUIContainer);
}

bool BattleState::update(sf::Time) { return false; }

bool BattleState::handleEvent(const sf::Event &event) {
  mGUIContainer.handleEvent(event);
  return false;
}
