#include "../include/Application.hpp"

#include "../include/StateIdentifiers.hpp"
#include "../include/StateStack.hpp"

#include "../include/BattleState.hpp"
#include "../include/ChooseBattleState.hpp"
#include "../include/GameState.hpp"
#include "../include/MenuState.hpp"
#include "../include/PauseState.hpp"
#include "../include/SettingsState.hpp"
#include "../include/TitleState.hpp"
const sf::Time Application::TimePerFrame = sf::seconds(1.f / 60.f);

Application::Application()
    : mWindow(sf::VideoMode(640, 480), "Pokemon Game", sf::Style::Close),
      mTextures(), mFonts(), mPlayer(),
      mStateStack(State::Context(mWindow, mTextures, mFonts, mPlayer)),
      mStatisticsText(), mStatisticsUpdateTime(), mStatisticsNumFrames(0) {
  mWindow.setKeyRepeatEnabled(false);

  mFonts.load(Fonts::Main, "../source/Sansation.ttf"); //规范字体
  mTextures.load(Textures::TitleScreen, "../source/Textures/TitleScreen.png");
  mTextures.load(Textures::ButtonNormal, "../source/Textures/ButtonNormal.png");
  mTextures.load(Textures::ButtonSelected,
                 "../source/Textures/ButtonSelected.png");
  mTextures.load(Textures::ButtonPressed,
                 "../source/Textures/ButtonPressed.png");
  mTextures.load(Textures::ButtonNormal_small,
                 "../source/Textures/ButtonNormal_small.png");
  mTextures.load(Textures::ButtonSelected_small,
                 "../source/Textures/ButtonSelected_small.png");
  mTextures.load(Textures::ButtonPressed_small,
                 "../source/Textures/ButtonPressed_small.png");
  mTextures.load(Textures::TextSelected, "../source/Textures/TextSelected.png");
  mTextures.load(Textures::TextNormal, "../source/Textures/TextNormal.png");
  mStatisticsText.setFont(mFonts.get(Fonts::Main));
  mStatisticsText.setPosition(5.f, 5.f);
  mStatisticsText.setCharacterSize(10u);

  registerStates();
  mStateStack.pushState(States::Title);
}

void Application::run() {
  sf::Clock clock;
  sf::Time timeSinceLastUpdate = sf::Time::Zero;

  while (mWindow.isOpen()) {
    sf::Time dt = clock.restart();
    timeSinceLastUpdate += dt;
    while (timeSinceLastUpdate > TimePerFrame) {
      timeSinceLastUpdate -= TimePerFrame;

      processInput();
      update(TimePerFrame);

      // Check inside this loop, because stack might be empty before update()
      // call
      if (mStateStack.isEmpty())
        mWindow.close();
    }

    updateStatistics(dt);
    render();
  }
}

void Application::processInput() {
  sf::Event event;
  while (mWindow.pollEvent(event)) {
    mStateStack.handleEvent(event);

    if (event.type == sf::Event::Closed)
      mWindow.close();
  }
}

void Application::update(sf::Time dt) { mStateStack.update(dt); }

void Application::render() {
  mWindow.clear();

  mStateStack.draw();

  mWindow.setView(mWindow.getDefaultView());
  mWindow.draw(mStatisticsText);

  mWindow.display();
}

void Application::updateStatistics(sf::Time dt) {
  mStatisticsUpdateTime += dt;
  mStatisticsNumFrames += 1;
  if (mStatisticsUpdateTime >= sf::seconds(1.0f)) {
    mStatisticsText.setString(
        "Frames / Second = " + std::to_string(mStatisticsNumFrames) + "\n" +
        "Time / Update = " +
        std::to_string(mStatisticsUpdateTime.asMicroseconds() /
                       mStatisticsNumFrames) +
        "us");

    mStatisticsUpdateTime -= sf::seconds(1.0f);
    mStatisticsNumFrames = 0;
  }
}

void Application::registerStates() {
  mStateStack.registerState<TitleState>(States::Title);
  mStateStack.registerState<MenuState>(States::Menu);
  mStateStack.registerState<GameState>(States::Game);
  mStateStack.registerState<PauseState>(States::Pause);
  mStateStack.registerState<SettingsState>(States::Settings);
  mStateStack.registerState<BattleState>(States::Battle);
  mStateStack.registerState<ChooseBattleState>(States::ChooseBattle);
}
