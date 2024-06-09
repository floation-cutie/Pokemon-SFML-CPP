#ifndef BOOK_PARALLELTASK_HPP
#define BOOK_PARALLELTASK_HPP

#include <SFML/System/Clock.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Thread.hpp>

class ParallelTask {
public:
  ParallelTask();
  void execute();
  bool isFinished(); //主线程函数
  float getCompletion();

private:
  //实际运行函数
  void runTask();

private:
  sf::Thread mThread;
  bool mFinished;
  sf::Clock mElapsedTime;
  sf::Mutex mMutex;
};

#endif // BOOK_PARALLELTASK_HPP
