#include "../include/ChooseBattleState.hpp"
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
#include <unistd.h>

extern int connect_sock;

ChooseBattleState::ChooseBattleState(StateStack &stack, Context context)
    : State(stack, context), mtextures(), mGUIContainer(), ltPokemon(nullptr),
      rtPokemon(nullptr), ldPokemon(nullptr), rdPokemon(nullptr), buf(),
      pokemonids() {
  // 先初始化vecotr<id>为0
  // std::cout << "DEBUG:: START CHOOSING\n";
  buf = "getPokemonList\n";
  if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
    std::cerr << "Error writing to server." << std::endl;
    close(connect_sock); // 正确关闭socket
    exit(1);
  }
  std::cout << "getPokemonList Message sent" << std::endl;
  char recvBuf[BUF_LENGTH];
  memset(recvBuf, 0, BUF_LENGTH);
  if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
    std::cerr << "Error receive message from server" << std::endl;
    close(connect_sock);
    exit(1);
  }
  // 将每行格式为PokemonID : (id) ...的字符串得到的id存入pokemonids中
  std::string str(recvBuf);
  auto strs = Helper::split(str, '\n');
  for (int i = 0; i < strs.size(); i++) {
    auto temp = Helper::split(strs[i], ' ');
    pokemonids.push_back(std::stoi(temp[2]));
  }

  // 使用lambda表达式起别名
  auto isValidNumber = [](const char *str) { return Helper::isValidPort(str); };
  loadTextures();
  buildScene();
  sf::Vector2u windowSize(getContext().window->getSize());
  //提示信息
  auto hintLabel = std::make_shared<GUI::Label>("Choose your Pokemon and enemy",
                                                *context.fonts);
  hintLabel->setPosition(windowSize.x / 2.f - 300, 40);
  hintLabel->setCharSize(20);

  auto lefttopPokemonLabel =
      std::make_shared<GUI::Label>("Level: ", *context.fonts);
  lefttopPokemonLabel->setPosition(windowSize.x / 8.f - 40,
                                   windowSize.y / 4.f + 50);
  lefttopPokemonLabel->setCharSize(20);
  mGUIContainer.pack(lefttopPokemonLabel);

  auto lefttopPokemonLevel =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  lefttopPokemonLevel->setPosition(windowSize.x / 8.f - 50,
                                   windowSize.y / 4.f + 80);
  lefttopPokemonLevel->setText("");

  auto righttopPokemonLabel =
      std::make_shared<GUI::Label>("Level: ", *context.fonts);
  righttopPokemonLabel->setPosition(5 * windowSize.x / 8.f - 40,
                                    windowSize.y / 4.f + 50);
  righttopPokemonLabel->setCharSize(20);

  auto righttopPokemonLevel =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  righttopPokemonLevel->setPosition(5 * windowSize.x / 8.f - 50,
                                    windowSize.y / 4.f + 80);
  righttopPokemonLevel->setText("");

  auto leftdownPokemonLabel =
      std::make_shared<GUI::Label>("Level: ", *context.fonts);
  leftdownPokemonLabel->setPosition(windowSize.x / 8.f - 40,
                                    3 * windowSize.y / 4.f + 20);
  leftdownPokemonLabel->setCharSize(20);

  auto leftdownPokemonLevel =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  leftdownPokemonLevel->setPosition(windowSize.x / 8.f - 50,
                                    3 * windowSize.y / 4.f + 50);
  leftdownPokemonLevel->setText("");

  auto rightdownPokemonLabel =
      std::make_shared<GUI::Label>("Level: ", *context.fonts);
  rightdownPokemonLabel->setPosition(5 * windowSize.x / 8.f - 40,
                                     3 * windowSize.y / 4.f + 20);
  rightdownPokemonLabel->setCharSize(20);

  auto rightdownPokemonLevel =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  rightdownPokemonLevel->setPosition(5 * windowSize.x / 8.f - 50,
                                     3 * windowSize.y / 4.f + 50);
  rightdownPokemonLevel->setText("");
  // --- ----- -- - ----------- - - -------- - 右侧的按钮
  auto PokemonIdInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  PokemonIdInputField->setPosition(530, 100);
  PokemonIdInputField->setText("Pokemon ID");

  auto autoFightButton =
      std::make_shared<GUI::SmallButton>(*context.fonts, *context.textures);
  autoFightButton->setPosition(530, 200);
  autoFightButton->setText("Auto Fight");
  autoFightButton->setCallback([=, this]() {
    if (autoFightButton->getText() == "Auto Fight") {
      autoFightButton->setText("Manual Fight");
    } else {
      autoFightButton->setText("Auto Fight");
    }
  });

  auto DuelFightButton =
      std::make_shared<GUI::SmallButton>(*context.fonts, *context.textures);
  DuelFightButton->setPosition(530, 250);
  DuelFightButton->setText("Duel Fight");
  DuelFightButton->setCallback([=, this]() {
    // 判断PokemonIdInputField中的输入是否在pokemonids中
    int Pokemonid = isValidNumber(PokemonIdInputField->getText().c_str());
    if (std::find(pokemonids.begin(), pokemonids.end(), Pokemonid) ==
        pokemonids.end()) {
      hintLabel->setText("The pokemon is not belong to you");
      hintLabel->setColor(sf::Color::Red);
      return;
    }
    // 发送请求
    buf = "Battle\nduel\n" + PokemonIdInputField->getText() + "\n";
    //
    //   判断四个精灵的输入框中是否只有一个输入框有等级输入，其他输入框均为空串
    // 等级输入且是否合法 为1 - 15之间的整数
    // 如果合法则发送请求
    if (lefttopPokemonLevel->getText() != "" &&
        righttopPokemonLevel->getText() == "" &&
        leftdownPokemonLevel->getText() == "" &&
        rightdownPokemonLevel->getText() == "") {
      int level = isValidNumber(lefttopPokemonLevel->getText().c_str());
      if (level < 1 || level > 15) {
        hintLabel->setText("Level should be in range 1 - 15");
        hintLabel->setColor(sf::Color::Red);
        return;
      }
      buf += "0\n" + std::to_string(level) + "\n";
    } else if (lefttopPokemonLevel->getText() == "" &&
               righttopPokemonLevel->getText() != "" &&
               leftdownPokemonLevel->getText() == "" &&
               rightdownPokemonLevel->getText() == "") {
      int level = isValidNumber(righttopPokemonLevel->getText().c_str());
      if (level < 1 || level > 15) {
        hintLabel->setText("Level should be in range 1 - 15");
        hintLabel->setColor(sf::Color::Red);
        return;
      }
      buf += "1\n" + std::to_string(level) + "\n";
    } else if (lefttopPokemonLevel->getText() == "" &&
               righttopPokemonLevel->getText() == "" &&
               leftdownPokemonLevel->getText() != "" &&
               rightdownPokemonLevel->getText() == "") {
      int level = isValidNumber(leftdownPokemonLevel->getText().c_str());
      if (level < 1 || level > 15) {
        hintLabel->setText("Level should be in range 1 - 15");
        hintLabel->setColor(sf::Color::Red);
        return;
      }
      buf += "2\n" + std::to_string(level) + "\n";
    } else if (lefttopPokemonLevel->getText() == "" &&
               righttopPokemonLevel->getText() == "" &&
               leftdownPokemonLevel->getText() == "" &&
               rightdownPokemonLevel->getText() != "") {
      int level = isValidNumber(rightdownPokemonLevel->getText().c_str());
      if (level < 1 || level > 15) {
        hintLabel->setText("Level should be in range 1 - 15");
        hintLabel->setColor(sf::Color::Red);
        return;
      }
      buf += "3\n" + std::to_string(level) + "\n";
    } else {
      hintLabel->setText("Only one Pokemon should have level input");
      hintLabel->setColor(sf::Color::Red);
      return;
    }

    // 判断autoFightButton的状态
    if (autoFightButton->getText() == "Auto Fight") {
      buf += "1\n";
    } else {
      buf += "0\n";
    }
    // -------------------- 现在已经处理完了buf格式 ------------

    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "duel Message sent" << std::endl;
    requestStackPop();
    requestStackPush(States::Battle);
  });

  // 升级赛按钮
  auto UpgradeFightButton =
      std::make_shared<GUI::SmallButton>(*context.fonts, *context.textures);
  UpgradeFightButton->setPosition(530, 300);
  UpgradeFightButton->setText("Upgrade Fight");
  UpgradeFightButton->setCallback([=, this]() {
    // 判断PokemonIdInputField中的输入是否在pokemonids中
    int Pokemonid = isValidNumber(PokemonIdInputField->getText().c_str());
    if (std::find(pokemonids.begin(), pokemonids.end(), Pokemonid) ==
        pokemonids.end()) {
      hintLabel->setText("The pokemon is not belong to you");
      hintLabel->setColor(sf::Color::Red);
      return;
    }
    // 发送请求
    buf = "Battle\nupgrade\n" + PokemonIdInputField->getText() + "\n";
    //
    //  判断四个精灵的输入框中是否只有一个输入框有等级输入，其他输入框均为空串
    // 等级输入且是否合法 为0 - 15之间的整数
    // 如果合法则发送请求
    if (lefttopPokemonLevel->getText() != "" &&
        righttopPokemonLevel->getText() == "" &&
        leftdownPokemonLevel->getText() == "" &&
        rightdownPokemonLevel->getText() == "") {
      int level = isValidNumber(lefttopPokemonLevel->getText().c_str());
      if (level < 1 || level > 15) {
        hintLabel->setText("Level should be in range 1 - 15");
        hintLabel->setColor(sf::Color::Red);
        return;
      }
      buf += "0\n" + std::to_string(level) + "\n";
    } else if (lefttopPokemonLevel->getText() == "" &&
               righttopPokemonLevel->getText() != "" &&
               leftdownPokemonLevel->getText() == "" &&
               rightdownPokemonLevel->getText() == "") {
      int level = isValidNumber(righttopPokemonLevel->getText().c_str());
      if (level < 1 || level > 15) {
        hintLabel->setText("Level should be in range 1 - 15");
        hintLabel->setColor(sf::Color::Red);
        return;
      }
      buf += "1\n" + std::to_string(level) + "\n";
    } else if (lefttopPokemonLevel->getText() == "" &&
               righttopPokemonLevel->getText() == "" &&
               leftdownPokemonLevel->getText() != "" &&
               rightdownPokemonLevel->getText() == "") {
      int level = isValidNumber(leftdownPokemonLevel->getText().c_str());
      if (level < 1 || level > 15) {
        hintLabel->setText("Level should be in range 1 - 15");
        hintLabel->setColor(sf::Color::Red);
        return;
      }
      buf += "2\n" + std::to_string(level) + "\n";
    } else if (lefttopPokemonLevel->getText() == "" &&
               righttopPokemonLevel->getText() == "" &&
               leftdownPokemonLevel->getText() == "" &&
               rightdownPokemonLevel->getText() != "") {
      int level = isValidNumber(rightdownPokemonLevel->getText().c_str());
      if (level < 1 || level > 15) {
        hintLabel->setText("Level should be in range 1 - 15");
        hintLabel->setColor(sf::Color::Red);
        return;
      }
      buf += "3\n" + std::to_string(level) + "\n";
    } else {
      hintLabel->setText("Only one Pokemon should have level input");
      hintLabel->setColor(sf::Color::Red);
      return;
    }

    // 判断autoFightButton的状态
    // 1 为自动战斗
    // 0 为手动战斗

    if (autoFightButton->getText() == "Auto Fight") {
      buf += "1\n";
    } else {
      buf += "0\n";
    }
    // -------------------- 现在已经处理完了buf格式 ------------

    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "upgrade Message sent" << std::endl;
    requestStackPop();
    requestStackPush(States::Battle);
  });
  // 查看自己的精灵列表
  auto PokemonListButton =
      std::make_shared<GUI::SmallButton>(*context.fonts, *context.textures);
  PokemonListButton->setPosition(530, 350);
  PokemonListButton->setText("Get PokemonList");
  PokemonListButton->setCallback([=, this]() {
    buf = "getPokemonList\n";
    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "getPokemonList Message sent" << std::endl;
    char recvBuf[BUF_LENGTH];
    memset(recvBuf, 0, BUF_LENGTH);
    if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(connect_sock);
      exit(1);
    }
    // 输入到控制台上
    std::cout << "receive PokemonList..\n";
    std::cout << recvBuf << std::endl;
    memset(recvBuf, 0, BUF_LENGTH);
  });

  // 退出按钮
  auto exitButton =
      std::make_shared<GUI::SmallButton>(*context.fonts, *context.textures);
  exitButton->setPosition(530, 400);
  exitButton->setText("Exit");
  exitButton->setCallback([=, this]() { requestStackPop(); });

  mGUIContainer.pack(hintLabel);
  mGUIContainer.pack(lefttopPokemonLabel);
  mGUIContainer.pack(lefttopPokemonLevel);
  mGUIContainer.pack(righttopPokemonLabel);
  mGUIContainer.pack(righttopPokemonLevel);
  mGUIContainer.pack(leftdownPokemonLabel);
  mGUIContainer.pack(leftdownPokemonLevel);
  mGUIContainer.pack(rightdownPokemonLabel);
  mGUIContainer.pack(rightdownPokemonLevel);
  mGUIContainer.pack(PokemonIdInputField);
  mGUIContainer.pack(autoFightButton);
  mGUIContainer.pack(DuelFightButton);
  mGUIContainer.pack(UpgradeFightButton);
  mGUIContainer.pack(PokemonListButton);
  mGUIContainer.pack(exitButton);
  // * /
}

void ChooseBattleState::loadTextures() {
  mtextures.load(Textures::BattleBackground,
                 "../source/Textures/battlebgForest.png");
  mtextures.load(Textures::defensive, "../source/Textures/defensive_small.png");
  mtextures.load(Textures::agile, "../source/Textures/agile_small.png");
  mtextures.load(Textures::strength, "../source/Textures/strength_small.png");
  mtextures.load(Textures::tank, "../source/Textures/tank_small.png");
}

void ChooseBattleState::buildScene() {
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
  // 地图的四角各放置一个不同类型的精灵以供选择,同时对应的图像下方给出精灵等级的Textfield
  auto lefttopPokemon = std::make_unique<components::spirits::Pokemon>(
      components::spirits::Pokemon::Species::strength, std::string("lt"),
      mtextures);
  ltPokemon = lefttopPokemon.get();
  lefttopPokemon->setPosition(windowSize.x / 8.f, windowSize.y / 4.f);
  mLayers[Spirits]->attachChild(std::move(lefttopPokemon));

  auto righttopPokemon = std::make_unique<components::spirits::Pokemon>(
      components::spirits::Pokemon::Species::tank, std::string("rt"),
      mtextures);
  rtPokemon = righttopPokemon.get();
  righttopPokemon->setPosition(5 * windowSize.x / 8.f, windowSize.y / 4.f);
  mLayers[Spirits]->attachChild(std::move(righttopPokemon));

  auto leftdownPokemon = std::make_unique<components::spirits::Pokemon>(
      components::spirits::Pokemon::Species::defensive, std::string("ld"),
      mtextures);
  ldPokemon = leftdownPokemon.get();
  leftdownPokemon->setPosition(windowSize.x / 8.f, 5 * windowSize.y / 8.f);
  mLayers[Spirits]->attachChild(std::move(leftdownPokemon));

  auto rightdownPokemon = std::make_unique<components::spirits::Pokemon>(
      components::spirits::Pokemon::Species::agile, std::string("rd"),
      mtextures);
  rdPokemon = rightdownPokemon.get();
  rightdownPokemon->setPosition(5 * windowSize.x / 8.f, 5 * windowSize.y / 8.f);
  mLayers[Spirits]->attachChild(std::move(rightdownPokemon));
}

void ChooseBattleState::draw() {
  sf::RenderWindow &window = *getContext().window;
  window.setView(window.getDefaultView());

  window.draw(mSceneGraph);
  window.draw(mGUIContainer);
}

bool ChooseBattleState::update(sf::Time) { return false; }

bool ChooseBattleState::handleEvent(const sf::Event &event) {
  mGUIContainer.handleEvent(event);
  return false;
}
