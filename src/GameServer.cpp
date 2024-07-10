#include "../include/GameServer.hpp"
#include "../include/HelperClass.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
GameServer &GameServer::getInstance() {
  static GameServer result; // singleton
  return result;
}

GameServer::~GameServer() {
  // all endpoints should be destroyed in Hub::start()
  // add these lines here just in case :)
  mtx.lock();
  while (endpoints.size()) {
    endpoints.pop_back();
  }
  mtx.unlock();
  /*
   * jthread会自动在解构函数中调用join()函数
  for (auto &th : pool) {
    th.join();
  }
  */
  close(serverSocket); // if socket has been closed, return WSAENOTSOCK, but
                       // that's ok
  close(connectSocket);
}

void GameServer::start() {
  std::cout << "Server: Init database TABLE...";
  if (sqlite3_open("../Data/server.db", &ppdb) != SQLITE_OK) {
    std::cerr << "\nServer: Can NOT open database: " << sqlite3_errmsg(ppdb)
              << std::endl;
    return;
  }
  // create tables they are not exist
  char *errMsg;
  string sql = "create table if not exists User(";
  sql += "id integer primary key,";
  sql += "firstLogin int not null,";
  sql += "name text unique not null,";
  sql += "password text not null,";
  sql += "win int not null,";
  sql += "total int not null";
  sql += ");";
  if (sqlite3_exec(ppdb, sql.c_str(), nonUseCallback, NULL, &errMsg) !=
      SQLITE_OK) {
    sqlite3_free(errMsg);
  }
  sql = "create table if not exists Pokemon(";
  sql += "id integer primary key,";
  sql += "userid integer not null,";
  sql += "specie int not null,";
  sql += "name text not null,";
  sql += "atk int not null,";
  sql += "def int not null,";
  sql += "maxHp int not null,";
  sql += "attackgap int not null,";
  sql += "lv int not null,";
  sql += "exp int not null,";
  sql += "pp int not null";
  sql += ");";
  if (sqlite3_exec(ppdb, sql.c_str(), nonUseCallback, NULL, &errMsg) !=
      SQLITE_OK) {
    //用于释放通过SQLite API分配的内存
    sqlite3_free(errMsg);
  }
  cout << "Done.\n";

  /**
   * init server socket
   */
  cout << "Server: Init server socket...";
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    std::cerr << "Error creating socket." << std::endl;
    return;
  }
  cout << "Done.\n";
  // 设置socket为非阻塞模式
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
             sizeof(opt));

  // construct an address, including protocol & IP address & port
  // 绑定socket到指定的端口
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(Server_Port);

  cout << "Server: Socket binding...";
  if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    cerr << "\nServer: Socket bind failed.\n";
    close(serverSocket);
    return;
  }
  cout << "Done.\n";

  // if request queue is full, client will get error: WSAECONNREFUSED
  // 改进：等待队列为满时，客户端应收到通知
  cout << "Server: Socket listen...";
  if (listen(serverSocket, REQ_QUEUE_LENGTH) < 0) {
    cout << "\nServer: Socket listen failed.\n";
    close(serverSocket);
    return;
  }
  cout << "Done.\n";

  // now listen successfully
  cout << "\nserver: server is running at port " << Server_Port << endl;
  cout << "Press any key to stop server.\n\n";

  // init thread
  running = true;
  // 绑定类中的成员函数到对应的对象中
  thread listenThread(&GameServer::listenFunc, this);
  thread terminateThread(&GameServer::terminateFunc, this);
  listenThread.join();
  terminateThread.join();

  // destroy all endpoints
  // 主线程中使用互斥锁
  std::unique_lock<mutex> lock(mtx);

  while (endpoints.size()) {
    endpoints.pop_back();
  }
  lock.unlock();

  close(serverSocket);

  sqlite3_close(ppdb);

  cout << "\nServer: Server stopped.\n";
}

void GameServer::listenFunc() {

  while (running) {
    // link
    cout << "Listening..." << endl;
    struct sockaddr_in clientAddr; // client address
    int clientAddrLength = sizeof(clientAddr);
    connectSocket = ::accept(serverSocket, (sockaddr *)&clientAddr,
                             (socklen_t *)&clientAddrLength);
    if (connectSocket < 0) {
      if (running) {
        // if not running, this thread must be terminated by terminateFunc
        // in that case the string below is not needed
        cout << "Server: Link to client failed.\n";
      }
      close(connectSocket);
      break;
    }

    // link successfully
    cout << "Server: client " << inet_ntoa(clientAddr.sin_addr)
         << " connected.\n";
    /**
     * process data
     * format:
     * - "login\n<username>\n<password>"
     * - "register\n<username>\n<password>"
     */
    memset(buf, 0, sizeof(buf));
    recv(connectSocket, buf, BUF_LENGTH, 0);

    auto strs = Helper::split(buf, '\n');
    for (auto str : strs)
      cout << "\nThe str is " << str << endl;
    if (strs.empty()) {
      continue;
    } else if (strs.size() < 3) {
      cout << "Server: Invalid request: " << buf << endl;
      strcpy(buf, "Reject: Invalid request.\n");
      send(connectSocket, buf, BUF_LENGTH, 0);
    } else if (strs[0] == "login")
      login(strs[1], strs[2]);
    else if (strs[0] == "register")
      Register(strs[1], strs[2]);
    else {
      cout << "Hub: Invalid request: " << buf << endl;
      strcpy(buf, "Reject: Invalid request.\n");
      send(connectSocket, buf, BUF_LENGTH, 0);
    }
    close(connectSocket);
  }
  cout << "Stop Listenning" << endl;
}

void GameServer::terminateFunc() {
  // press any key to stop server
  //  std::cout << "The terminateFunc thread started..\n";
  getchar();

  running = false;
  std::cout << "The GameServer now has terminated..\n";
  close(serverSocket); // stop listenning
}

bool GameServer::isValid(const string &str) {
  if (str[0] == '_')
    return false;
  for (auto c : str) {
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_')) {
      // not a letter or a digit or '_'
      return false;
    }
  }
  return true;
}

// 向客户端发送长连接的端口号消息
void GameServer::login(const string &username, const string &password) {
  if (!isValid(username)) {
    cout << "Server: Got an invalid username: " << username << endl;
    strcpy(buf, "Reject: Invalid username.\n");
  } else if (!isValid(password)) {
    cout << "Server: Got an invalid password: " << password << endl;
    strcpy(buf, "Reject: Invalid password.\n");
  } else {
    char **sqlResult;
    int nRow;
    int nColumn;
    char *errMsg;
    string sql = "SELECT id FROM User WHERE name = '" + username +
                 "' AND password = '" + password + "'";
    if (sqlite3_get_table(ppdb, sql.c_str(), &sqlResult, &nRow, &nColumn,
                          &errMsg) != SQLITE_OK) {
      cout << "Server: Sqlite3 search error: " << errMsg << endl;
      strcpy(buf, "The database hasn't generated normally.\n");
      sqlite3_free(errMsg);
    } else // sqlite select succeed
    {
      if (nRow == 0) {
        // username and password mismatch
        cout << "Server: Login: username '" << username << "' and password '"
             << password << "' mismatch.\n";
        strcpy(buf, "The username or password is wrong.\n");
      } else {
        // check user state
        bool userExist = false;
        std::unique_lock<mutex> lock(mtx);
        //将参数字符串转换为整数
        int id = atoi(sqlResult[1]);
        std::cout << "the id is " << id << std::endl;
        std::cout << "the endpoint has " << endpoints.size() << " clients"
                  << std::endl;
        if (endpoints.size() > 0)
          std::cout << "the endpoint is" << endpoints[0]->getPlayerID()
                    << std::endl;
        // sqlResult[0] == "id", sqlResult[1] == playerID(实际ID值)
        for (auto endpoint : endpoints) {
          if (endpoint->getPlayerID() == id) {
            userExist = true;
            if (endpoint->isOnline()) {
              strcpy(buf, "Reject: Account is already online.\n");
            } else {
              // not online, return port
              strcpy(buf, to_string(endpoint->getPort()).c_str());
            }
            break;
          }
        }
        lock.unlock();

        //对于首次注册,还未登录长连接的用户
        if (!userExist) // add an endpoint
        {
          auto p = make_shared<Endpoint>(id, ppdb, *this);
          int endpointPort = p->start();
          if (endpointPort == 0) // start ERROR
          {
            strcpy(buf, "Reject: Server endpoint error.\n");
          } else // start normally, add this endpoint to endpoints
          {
            lock_guard<mutex> lock(mtx);
            endpoints.push_back(p);

            std::cout << "the endpoint has " << endpoints.size() << " clients"
                      << std::endl;
            std::cout << "And the id is " << endpoints[0]->getPlayerID()
                      << std::endl;
            strcpy(buf, to_string(endpointPort).c_str());
            std::jthread th(&GameServer::monitor, this, p);
            pool.push_back(std::move(th)); //放入全局的线程池
          }
        }
      }
      sqlite3_free_table(sqlResult);
    }
  }
  send(connectSocket, buf, BUF_LENGTH, 0);
}

void GameServer::Register(const string &username, const string &password) {
  if (!isValid(username)) {
    cout << "Server: Got an invalid username: " << username << endl;
    strcpy(buf, "Reject: Invalid username.\n");
  } else if (!isValid(password)) {
    cout << "Server: Got an invalid password: " << password << endl;
    strcpy(buf, "Reject: Invalid password.\n");
  } else {
    char **sqlResult;
    int nRow;
    int nColumn;
    char *errMsg;
    string sql = "SELECT name FROM User WHERE name = '" + username + "'";
    if (sqlite3_get_table(ppdb, sql.c_str(), &sqlResult, &nRow, &nColumn,
                          &errMsg) != SQLITE_OK) {
      cout << "Server: Sqlite3 error: " << errMsg << endl;
      // strcpy(buf, "Reject: Hub database error.\n");
      strcpy(buf, "The database hasn't generated normally.\n");
      sqlite3_free(errMsg);
    } else {
      if (nRow == 0) {
        // username NOT exist, add this user with five information
        string sql = "INSERT INTO User(firstLogin, name, password, win, total) "
                     "VALUES(1, '" +
                     username + "', '" + password + "', 0, 0);";
        char *errMsg;
        if (sqlite3_exec(ppdb, sql.c_str(), nonUseCallback, NULL, &errMsg) !=
            SQLITE_OK) {
          cout << "Server: Sqlite3 error: " << errMsg << endl;
          sqlite3_free(errMsg);
          strcpy(buf, "The database hasn't generated normally.\n");
        } else {
          cout << "Server: Add user: " << username << " password: " << password
               << endl;
          strcpy(buf, "Accept.\n");
        }
      } else {
        // username already exist
        cout << "Server: Register: username '" << username
             << "' already exist.\n";
        strcpy(buf, "Reject: Duplicate username.\n");
      }
      sqlite3_free_table(sqlResult);
    }
  }
  send(connectSocket, buf, BUF_LENGTH, 0);
}

void GameServer::monitor(shared_ptr<Endpoint> const endpoint) {
  std::cout << "Starting monitor endpoint " << endpoint->getPlayerID()
            << "...\n";
  endpoint->process();
  // now endpoint reaches end
  std::unique_lock<mutex> lock(mtx);
  // remove from endpoints
  for (int i = 0; i < endpoints.size(); ++i) {
    if (endpoints[i] == endpoint) {
      endpoints.erase(endpoints.begin() + i);
      break;
    }
  }
  std::cout << "Stop monitoring..\n";
  lock.unlock();
}

/**
 * format:
 * <userID> <userName> <online|offline>
 * 查询所有用户和状态
 */
string GameServer::getAllUser() {
  struct temp {
    string name;
    bool online;
    string win;
    string total;
  };
  // get all user
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  string sql = "SELECT id, name, win, total FROM User;";
  if (sqlite3_get_table(ppdb, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Server: Sqlite3 error: " << errMsg << endl;
    // strcpy(buf, "Reject: Hub database error.\n");
    sqlite3_free(errMsg);
  }

  // construct playerMap
  map<int, temp> playerMap;
  for (int i = 0; i < nRow; ++i) {
    temp t = {sqlResult[4 * (i + 1) + 1], false, sqlResult[4 * (i + 1) + 2],
              sqlResult[4 * (i + 1) + 3]};
    playerMap.insert(make_pair(stoi(sqlResult[4 * (i + 1)]), t));
  }

  sqlite3_free_table(sqlResult);

  // judge user online and put online user first
  string result;
  unique_lock<mutex> lock(mtx);
  for (auto endpoint : endpoints) {
    playerMap[endpoint->getPlayerID()].online = true;
  }
  mtx.unlock();

  for (auto &player : playerMap) {
    if (player.second.online) {
      result = "PlayerID: " + to_string(player.first) +
               " PlayerName: " + player.second.name + " online " +
               "win: " + player.second.win +
               " total fight: " + player.second.total + "\n" + result;
    } else {
      result += "PlayerID: " + to_string(player.first) +
                " PlayerName: " + player.second.name + " offline " +
                "win: " + player.second.win +
                " total fight: " + player.second.total + "\n";
    }
  }

  return result;
}
