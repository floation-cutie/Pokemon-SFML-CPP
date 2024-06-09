#include "../include/PauseState.hpp"
#include "../include/Button.hpp"
#include "../include/HelperClass.hpp"
#include "../include/Label.hpp"
#include "../include/NetworkProtocol.hpp"
#include "../include/ResourceHolder.hpp"
#include "../include/Textfield.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <arpa/inet.h>
#include <cmath>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
extern int connect_sock;

PauseState::PauseState(StateStack &stack, Context context)
    : State(stack, context), mBackgroundSprite(), mPausedText(),
      mGUIContainer() {
  sf::Font &font = context.fonts->get(Fonts::Main);
  sf::Vector2f windowSize(context.window->getSize());

  mPausedText.setFont(font);
  mPausedText.setString("Game Paused");
  mPausedText.setCharacterSize(70);
  sf::FloatRect bounds = mPausedText.getLocalBounds();
  mPausedText.setOrigin(std::floor(bounds.left + bounds.width / 2.f),
                        std::floor(bounds.top + bounds.height / 2.f));
  mPausedText.setPosition(0.5f * windowSize.x, 0.05f * windowSize.y);

  auto returnButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  returnButton->setPosition(0.5f * windowSize.x - 100,
                            0.4f * windowSize.y - 110);
  returnButton->setText("Return");
  returnButton->setCallback([this]() { requestStackPop(); });

  auto PlayIdInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  PlayIdInputField->setPosition(0.5f * windowSize.x - 200,
                                0.4 * windowSize.y - 60);
  PlayIdInputField->setText("");
  auto PokemonBagButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  PokemonBagButton->setPosition(0.5f * windowSize.x - 100,
                                0.4f * windowSize.y - 60);
  PokemonBagButton->setText("Check Pokemons BY PlayerID");
  PokemonBagButton->setCallback([=, this]() {
    if (PlayIdInputField->getText() == "") {
      buf = "getPokemonList\n";
    } else {
      buf = "getPokemonList\n" + PlayIdInputField->getText() + "\n";
    }
    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "getPokemonList Message sent" << std::endl;
    char recvBuf[BUF_LENGTH] = "";
    if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(connect_sock);
      exit(1);
    }
    std::cout << "receive PokemonList..\n";
    std::cout << recvBuf << std::endl;
  });

  auto PlayerListButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  PlayerListButton->setPosition(0.5f * windowSize.x - 100,
                                0.4f * windowSize.y - 10);
  PlayerListButton->setText("Get PlayerList");
  PlayerListButton->setCallback([=, this]() {
    buf = "getPlayerList\n";
    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "getPlayerList Message sent" << std::endl;
    char recvBuf[BUF_LENGTH] = "";
    if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(connect_sock);
      exit(1);
    }
    std::cout << "receive PlayerList..\n";
    std::cout << recvBuf << std::endl;
  });

  auto PokemonIdInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  PokemonIdInputField->setPosition(0.5f * windowSize.x - 200,
                                   0.4 * windowSize.y + 40);
  PokemonIdInputField->setText("Pokemon ID");
  auto GetPokemonButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  GetPokemonButton->setPosition(0.5f * windowSize.x - 100,
                                0.4f * windowSize.y + 40);
  GetPokemonButton->setText("Check Pokemon BY Pokemon ID");
  GetPokemonButton->setCallback([=, this]() {
    buf = "getPokemon\n" + PokemonIdInputField->getText() + "\n";
    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "getPokemon Message sent" << std::endl;
    char recvBuf[BUF_LENGTH] = "";
    if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(connect_sock);
      exit(1);
    }
    std::cout << "receive Pokemon Information..\n";
    std::cout << recvBuf << std::endl;
  });

  auto oldPdInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  oldPdInputField->setPosition(0.5f * windowSize.x - 305,
                               0.4 * windowSize.y + 90);
  oldPdInputField->setText("OldPassword");
  auto newPdInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  newPdInputField->setPosition(0.5f * windowSize.x - 200,
                               0.4 * windowSize.y + 90);
  newPdInputField->setText("NewPassword");
  auto ChangePdButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  ChangePdButton->setPosition(0.5f * windowSize.x - 100,
                              0.4f * windowSize.y + 90);
  ChangePdButton->setText("Change Password");
  ChangePdButton->setCallback([=, this]() {
    std::cout << "The old text input is " << oldPdInputField->getText()
              << std::endl;
    std::cout << "The new text input is " << newPdInputField->getText()
              << std::endl;

    buf = "resetPassword\n" + oldPdInputField->getText() + "\n" +
          newPdInputField->getText() + "\n";
    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    char recvBuf[BUF_LENGTH] = "";
    if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(connect_sock);
      exit(1);
    }
    std::cout << "changing password..\n";
    std::cout << recvBuf << std::endl;
  });

  auto ChangePoIdInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  ChangePoIdInputField->setPosition(0.5f * windowSize.x - 305,
                                    0.4 * windowSize.y + 140);
  ChangePoIdInputField->setText("Pokemon ID");
  auto newNameInputField =
      std::make_shared<GUI::Textfield>(*context.fonts, *context.textures);
  newNameInputField->setPosition(0.5f * windowSize.x - 200,
                                 0.4 * windowSize.y + 140);
  newNameInputField->setText("NewName");
  auto ChangePoNameButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  ChangePoNameButton->setPosition(0.5f * windowSize.x - 100,
                                  0.4f * windowSize.y + 140);
  ChangePoNameButton->setText("Change pokemonName");
  ChangePoNameButton->setCallback([=, this]() {
    buf = "pokemonChangeName\n" + ChangePoIdInputField->getText() + "\n" +
          newNameInputField->getText() + "\n";
    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    char recvBuf[BUF_LENGTH] = "";
    if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(connect_sock);
      exit(1);
    }
    std::cout << "changing pokemon name..\n";
    std::cout << recvBuf << std::endl;
  });

  //查看用户荣誉按钮
  auto getHonorButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  getHonorButton->setPosition(0.5f * windowSize.x - 100,
                              0.4f * windowSize.y + 190);
  getHonorButton->setText("Get Honor");
  getHonorButton->setCallback([=, this]() {
    buf = "getUserHonor";
    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "getHonor Message sent" << std::endl;
    char recvBuf[BUF_LENGTH] = "";
    if (read(connect_sock, recvBuf, BUF_LENGTH) < 0) {
      std::cerr << "Error receive message from server" << std::endl;
      close(connect_sock);
      exit(1);
    }
    auto recvStr = std::string(recvBuf);
    auto strs = Helper::split(recvStr, '\n');
    std::cout << "You are a Pokemon Foster(judge by their level)" << strs[0]
              << std::endl
              << "You are a Pokemon Trainer(judge by their number)" << strs[1]
              << std::endl;
  });

  auto backToMenuButton =
      std::make_shared<GUI::Button>(*context.fonts, *context.textures);
  backToMenuButton->setPosition(0.5f * windowSize.x - 100,
                                0.4f * windowSize.y + 240);
  backToMenuButton->setText("Logout");
  backToMenuButton->setCallback([this]() {
    buf = "logout\n";
    if (send(connect_sock, buf.c_str(), buf.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connect_sock); // 正确关闭socket
      exit(1);
    }
    std::cout << "Logout Message sent" << std::endl;
    requestStateClear();
    requestStackPush(States::Menu);
  });

  mGUIContainer.pack(returnButton);
  mGUIContainer.pack(PlayIdInputField);
  mGUIContainer.pack(PokemonBagButton);
  mGUIContainer.pack(PlayerListButton);
  mGUIContainer.pack(PokemonIdInputField);
  mGUIContainer.pack(GetPokemonButton);
  mGUIContainer.pack(oldPdInputField);
  mGUIContainer.pack(newPdInputField);
  mGUIContainer.pack(ChangePdButton);
  mGUIContainer.pack(ChangePoIdInputField);
  mGUIContainer.pack(newNameInputField);
  mGUIContainer.pack(ChangePoNameButton);
  mGUIContainer.pack(getHonorButton);
  mGUIContainer.pack(backToMenuButton);
}

void PauseState::draw() {
  sf::RenderWindow &window = *getContext().window;
  window.setView(window.getDefaultView());

  sf::RectangleShape backgroundShape;
  backgroundShape.setFillColor(sf::Color(0, 0, 0, 150));
  backgroundShape.setSize(window.getView().getSize());

  window.draw(backgroundShape);
  window.draw(mPausedText);
  window.draw(mGUIContainer);
}

bool PauseState::update(sf::Time) { return false; }

bool PauseState::handleEvent(const sf::Event &event) {
  mGUIContainer.handleEvent(event);
  return false;
}
