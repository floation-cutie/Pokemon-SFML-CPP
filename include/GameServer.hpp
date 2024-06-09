#pragma once

#include "../include/NetworkProtocol.hpp"
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <sqlite3.h>
#include <string>
#include <sys/socket.h>
#include <thread> // use c++20 standard
#include <vector>
// about endpoint
#include "../include/EndPoint.hpp"

using namespace std;

/**
 * define interfaces before login
 * use interfaces in class Endpoint after login
 *
 * this class is Singleton
 */
class GameServer {
private:
  // about network
  int serverSocket;
  int connectSocket;
  int opt = 1;
  struct sockaddr_in address;
  bool running;
  char buf[BUF_LENGTH];

  // about database
  sqlite3 *ppdb;

  // about endpoint
  vector<std::shared_ptr<Endpoint>> endpoints;

  // about thread
  mutex mtx; // to protect endpoints
  std::vector<std::jthread> pool;
  // interfaces
  void login(const string &username, const string &password);

  void Register(const string &username, const string &password);

  // authentication
  bool isValid(const string &str);
  // thread function
  void listenFunc();
  void terminateFunc();
  //用于断开对应的长连接
  void monitor(shared_ptr<Endpoint> const endpoint);

  // for singleton
  GameServer(){};
  GameServer(GameServer const &) = delete;
  GameServer(GameServer &&) = delete;
  GameServer &operator=(GameServer const &) = delete;
  GameServer &operator=(GameServer &&) = delete;
  ~GameServer();

public:
  //提供静态的工厂方法，确保一个类只会有一个实例，并提供一个全局访问点获取该示例
  static GameServer &getInstance();

  void start();        // init database and socket
  string getAllUser(); // for endpoint::getPlayerList
};

//可以用于DEBUG 打印表内信息
inline int nonUseCallback(void *notUsed, int argc, char **argv,
                          char **azColName) {
  // 输出查询结果
  return 0;
  for (int i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return SQLITE_OK; // 表示处理了当前行，继续处理下一行
  //  return 0;
}
