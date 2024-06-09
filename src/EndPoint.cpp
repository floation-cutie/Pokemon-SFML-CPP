#include "../include/EndPoint.hpp"
#include "../include/BattleController.hpp"
#include "../include/GameServer.hpp"
#include "../include/HelperClass.hpp"
#include "../include/Pokemon.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <netinet/in.h>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
using namespace std;

Endpoint::Endpoint(int playerID, sqlite3 *db, GameServer &server)
    : playerID(playerID), db(db), server(server) {
  port = 0;
  running = false;
}

Endpoint::~Endpoint() {
  running = false;
  while (timing) {
    // cout << "Try to stop timer.\n";
    unique_lock<mutex> lock(mtx);
    online = true;
    lock.unlock();
    //计时器判断
    cv.notify_one();
    lock.lock();
  }

  close(endpointSocket);

  if (port)
    cout << "Endpoint[" << playerID << "]: Endpoint stoped at " << port << endl;
}

int Endpoint::start() {
  // get playerUsername
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  string sql = "SELECT name FROM User where id=" + to_string(playerID) + ";";
  if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    sqlite3_free(errMsg);
    return 0;
  } else if (nRow == 0) {
    cout << "Endpoint[" << playerID << "]: Database content error.\n";
    sqlite3_free_table(sqlResult);
    return 0;
  } else {
    playerUsername = sqlResult[1];
    sqlite3_free_table(sqlResult);
  }

  // init endpoint socket
  endpointSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (endpointSocket < 0) {
    cout << "Endpoint[" << playerID << "]: Init socket failed.\n";
    close(endpointSocket);
    return 0;
  }

  // construct an address, including protocol & IP address & port
  sockaddr_in endpointAddr;
  endpointAddr.sin_family = AF_INET;
  endpointAddr.sin_port = htons(0); // 操作系统自动分配端口号
  endpointAddr.sin_addr.s_addr = htonl(INADDR_ANY); // any ip address

  // bind socket to an address
  if (bind(endpointSocket, (sockaddr *)&endpointAddr, sizeof(endpointAddr)) <
      0) {
    cerr << "Endpoint[" << playerID << "]: Socket bind failed.\n";
    close(endpointSocket);
    return 0;
  }

  // 获取实际端口号
  int endpointAddrLength = sizeof(endpointAddr);
  getsockname(endpointSocket, (struct sockaddr *)&endpointAddr,
              (socklen_t *)&endpointAddrLength);
  port = ntohs(endpointAddr.sin_port);

  // if request queue is full, 客户端需得到通知
  if (listen(endpointSocket, REQ_QUEUE_LENGTH) < 0) {
    cout << "Endpoint[" << playerID << "]: Socket listen failed.\n";
    close(endpointSocket);
    return 0;
  }

  cout << "Endpoint[" << playerID << "]: Endpoint is running at " << port
       << "\n";

  running = true; // enable Endpoint::process()

  return port;
}

//处理实际请求,进行accept recv send通信
void Endpoint::process() {
  while (running) {
    //首先假设用户离线
    online = false;
    timing = true;
    memset(buf, 0, sizeof(buf));
    thread timerThread(&Endpoint::timerFunc, this);
    thread listenThread(&Endpoint::listenFunc, this);
    timerThread.join();
    listenThread.join();
  }
}

// 单监听，整体监听放在process模块内部
void Endpoint::listenFunc() {
  // link
  cout << "start listening..\n";

  //产生阻塞，等待客户端的响应
  struct sockaddr_in clientAddr; // client address
  int clientAddrLength = sizeof(clientAddr);
  connectSocket = ::accept(endpointSocket, (sockaddr *)&clientAddr,
                           (socklen_t *)&clientAddrLength);
  while (timing) {
    cout << "Try to stop timer.\n";
    unique_lock<mutex> lock(mtx);
    online = true;
    lock.unlock();
    cv.notify_one();
    lock.lock();
  }
  if (connectSocket < 0) {
    // TODO
    return;
  }
  // link successfully
  int ret = recv(connectSocket, buf, BUF_LENGTH, 0);
  /**
   * recv(connSocket, buf, BUF_LENGTH, 0)
   * - return bytes of buf
   * - return 0 if client socket closed or get an empty line
   * - return SOCKET_ERROR(-1) if server socket is closed or client unexpectedly
   * terminated
   */
  while (ret != 0 && running) // normal
  {
    // parse command here

    auto strs = Helper::split(buf, '\n');

    for (auto str : strs) {
      cout << "The str is " << str << endl;
    }

    if (strs.empty()) {
    } else if (strs[0] == "logout") {
      running = false;
    } else if (strs[0] == "getPlayerList") {
      getPlayerList();
    } else if (strs[0] == "resetPassword" && strs.size() == 3) {
      resetPassword(strs[1], strs[2]);
    } else if (strs[0] == "getPokemonList" && strs.size() < 3) {
      if (strs.size() == 2) {
        getPokemonList(stoi(strs[1]));
      } else {
        getPokemonList(playerID);
      }
    } else if (strs[0] == "getPokemon" && strs.size() == 2) {
      getPokemonByID(stoi(strs[1]));
    } else if (strs[0] == "pokemonChangeName" && strs.size() == 3) {
      pokemonChangeName(strs[1], strs[2]);
    } else if (strs[0] == "Battle" && strs.size() == 6) {
      if (strs[1] == "duel")
        isDuel = true;
      else if (strs[1] == "upgrade")
        isDuel = false;
      battle(stoi(strs[2]), stoi(strs[3]), stoi(strs[4]), stoi(strs[5]));
    } else if (strs[0] == "discard" && strs.size() == 2) {
      discard(stoi(strs[1]));
    } else {
      cout << "Endpoint[" << playerID << "]: Invalid request.\n";
      strcpy(buf, "Reject: Invalid request.\n");
      send(connectSocket, buf, BUF_LENGTH, 0);
    }
    memset(buf, 0, sizeof(buf));
    if (running)
      ret = recv(connectSocket, buf, BUF_LENGTH, 0);
  }
  if (!running)
    ; // this object was destroyed by destructor
  else if (ret == 0) {
    cout << "Endpoint[" << playerID
         << "]: Client unexpected offline, start timing.\n";
  } else {
    // running == false, user logout
    cout << "Endpoint[" << playerID << "]: User logout.\n";
  }
  close(connectSocket);
}

void Endpoint::timerFunc() {
  /**
   * wait for player re-login for 1 minutes
   */
  unique_lock<mutex> lock(mtx);
  if (!cv.wait_for(lock, std::chrono::minutes(1), [this] { return online; })) {
    // player is offline
    cout << "player is offline\n";
    running = false;
    timing = false;
    close(endpointSocket);
  } else {
    cout << "player is still online\n";
    timing = false;
  }
}

void Endpoint::resetPassword(const string &oldPassword,
                             const string &newPassword) {
  // check oldPassword
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  string sql;
  sql = "SELECT name FROM User where id=" + to_string(playerID) +
        " and password='" + oldPassword + "';";
  if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
    return;
  } else if (nRow == 0) {
    // wrong password
    sqlite3_free_table(sqlResult);
    strcpy(buf, "Reject: wrong old password.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
    return;
  } else {
    sqlite3_free_table(sqlResult);
  }

  // update password
  sql = "update User set password='" + newPassword +
        "' where id=" + to_string(playerID) + ";";
  if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
    return;
  } else {
    sqlite3_free_table(sqlResult);
    strcpy(buf, "Accept.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
  }
  return;
}

void Endpoint::getPlayerList() {
  strcpy(buf, server.getAllUser().c_str());
  send(connectSocket, buf, BUF_LENGTH, 0);
}

void Endpoint::getPokemonList(int playerID) {
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  string sql;
  cout << "The playerID is " << playerID << endl;
  //  获取用户是否首次登录信息，放入isFirstLogin变量中
  string sql_login =
      "SELECT firstLogin FROM User where id=" + to_string(playerID) + ";";
  if (sqlite3_get_table(db, sql_login.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    cout << "TEST DEBUG" << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
    return;
  }
  bool isFirstLogin = sqlResult[1][0] == '1';

  //
  sql = "SELECT id, specie,name, lv FROM Pokemon where userid=" +
        to_string(playerID) + ";";
  if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    cout << "TEST DEBUG" << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
    return;
  }
  //首次登陆，随机送三只精灵
  // cout << "The row has " << nRow << " ...\n";
  if (playerID == this->playerID && isFirstLogin) {
    // add pokemons for user till his pokemon number be 3
    for (int i = 0; i < 3; ++i) {
      int Index = rand() % 4;
      auto t = make_shared<components::spirits::Pokemon>(
          static_cast<components::spirits::PokemonBase::Species>(Index),
          Name[Index]);
      cout << "Saving Pokemon\n";
      savePokemonToDB(t);
    }
    //  更新用户首次登录状态
    string sql_update =
        "update User set firstLogin = 0 where id=" + to_string(playerID) + ";";
    if (sqlite3_exec(db, sql_update.c_str(), nonUseCallback, NULL, &errMsg) !=
        SQLITE_OK) {
      cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
      cout << "Something wrong happened.\n";
      sqlite3_free(errMsg);
    }

    getPokemonList(playerID);
  } else {
    string result;
    //返回格式
    for (int i = 0; i < nRow; ++i) {
      result += "PokemonID : ";
      result += sqlResult[4 * (i + 1)]; // id
      result += " Specie : ";
      result += sqlResult[4 * (i + 1) + 1]; // specie
      result += " Name :";
      result += sqlResult[4 * (i + 1) + 2]; // name
      result += " Lv :";
      result += sqlResult[4 * (i + 1) + 3]; // lv
      result += '\n';
    }
    strcpy(buf, result.c_str());
    send(connectSocket, buf, BUF_LENGTH, 0);
  }
  sqlite3_free_table(sqlResult);
}

void Endpoint::savePokemonToDB(shared_ptr<components::spirits::Pokemon> p,
                               int id) {
  // Pokemon not exist,insert it to the database
  if (id == -1) {
    string sql = "INSERT INTO Pokemon(userid, specie, name, atk, def, maxHp, "
                 "attackgap, lv, exp, pp) VALUES('";
    sql += to_string(playerID) + "',";
    sql += to_string(p->GetSpecie()) + ",'";
    sql += p->GetName() + "',";
    sql += to_string(p->GetAttackPoint()) + ",";
    sql += to_string(p->GetDefensePoint()) + ",";
    sql += to_string(p->GetHealthPoint()) + ",";
    sql += to_string(p->GetAttackFrequence()) + ",";
    sql += to_string(p->GetLevel()) + ",";
    sql += to_string(p->GetExp()) + ",";
    sql += to_string(p->GetPowerPoint()) + ");";
    char *errMsg;
    if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) !=
        SQLITE_OK) {
      cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
      cout << "Something wrong happened.\n";
      sqlite3_free(errMsg);
    }
    cout << "Saving Pokemon into database\n";
  } else {
    string sql = "update Pokemon set atk=";
    sql += to_string(p->GetAttackPoint()) + ", def=";
    sql += to_string(p->GetDefensePoint()) + ", maxHp=";
    sql += to_string(p->GetHealthPoint()) + ", attackgap=";
    sql += to_string(p->GetAttackFrequence()) + ", lv=";
    sql += to_string(p->GetLevel()) + ", exp=";
    sql += to_string(p->GetExp()) + ", pp=";
    sql += to_string(p->GetPowerPoint());
    sql += " where id=";
    sql += to_string(id) + ";";
    char *errMsg;
    if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) !=
        SQLITE_OK) {
      cout << "The pokemon can't go into the database" << endl;
      cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
      sqlite3_free(errMsg);
    } else {
      cout << "Saving Pokemon into database\n";
    }
  }
}

void Endpoint::getPokemonByID(int pokemonID) {
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  string sql;
  sql = "SELECT id, specie, name, atk, def, maxHp, attackgap, lv, exp, pp FROM "
        "Pokemon "
        "where id=" +
        to_string(pokemonID) + ";";
  if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
    return;
  }
  //返回格式
  string result;
  result += " PokemonID :";
  result += sqlResult[10 + 0]; // id
  result += " Specie :";
  result += sqlResult[10 + 1]; // specie
  result += " Name :";
  result += sqlResult[10 + 2]; // name
  result += " Atk :";
  result += sqlResult[10 + 3]; // atk
  result += " Def :";
  result += sqlResult[10 + 4]; // def
  result += " MaxHp :";
  result += sqlResult[10 + 5]; // maxHp
  result += " AttackGap :";
  result += sqlResult[10 + 6]; // attackgap
  result += " Lv :";
  result += sqlResult[10 + 7]; // lv
  result += " Exp :";
  result += sqlResult[10 + 8]; // exp
  result += " PP :";
  result += sqlResult[10 + 9]; // pp
  result += '\n';
  strcpy(buf, result.c_str());
  send(connectSocket, buf, BUF_LENGTH, 0);
  sqlite3_free_table(sqlResult);
}

void Endpoint::pokemonChangeName(const string &pokemonID,
                                 const string &newName) {
  string sql_check = "SELECT userid FROM Pokemon where id = " + pokemonID + ";";

  string sql_update =
      "update Pokemon set name = '" + newName + "' where id=" + pokemonID + ";";
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  if (sqlite3_get_table(db, sql_check.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
  } else {
    string searchID = sqlResult[1];
    cout << "DEBUG :: The searched playerID is " << searchID << endl;
    if (searchID != to_string(playerID)) {
      strcpy(buf, "The Pokemon is not belong to you...\n");
      send(connectSocket, buf, BUF_LENGTH, 0);
    } else {
      if (sqlite3_exec(db, sql_update.c_str(), nonUseCallback, NULL, &errMsg) !=
          SQLITE_OK) {
        cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg
             << endl;
        strcpy(buf, errMsg);
        sqlite3_free(errMsg);
        send(connectSocket, buf, BUF_LENGTH, 0);
      } else {
        strcpy(buf, "Accept.\n");
        send(connectSocket, buf, BUF_LENGTH, 0);
      }
    }
  }
  sqlite3_free_table(sqlResult);
}

string Endpoint::chooseBet() {
  // get all pokemon's id for the player
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  string sql;
  sql = "SELECT id FROM Pokemon where userid=" + to_string(playerID) + ";";
  if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    sqlite3_free(errMsg);
    sqlite3_free_table(sqlResult);
    return "Reject: Server error.\n";
  }

  // 从中随机选择三个id，若不足三个则返回所有id，格式为”id1\nid2\nid3\n“
  int count = nRow - 1;
  if (count >= 3) {
    int index1 = rand() % count + 1;
    int index2 = rand() % count + 1;
    int index3 = rand() % count + 1;
    while (index1 == index2 || index1 == index3 || index2 == index3) {
      index1 = rand() % count + 1;
      index2 = rand() % count + 1;
      index3 = rand() % count + 1;
    }
    string result;
    result += sqlResult[index1];
    result += "\n";
    result += sqlResult[index2];
    result += "\n";
    result += sqlResult[index3];
    result += "\n";
    sqlite3_free_table(sqlResult);
    return result;
  } else {
    string result;
    for (int i = 1; i <= count; ++i) {
      result += sqlResult[i];
      result += "\n";
    }
    sqlite3_free_table(sqlResult);
    return result;
  }
}

// 先从数据库中将对应ID的精灵取出，将数据传给客户端，客户端进行虚拟战斗，
// 最后将战斗结果传给服务器，服务器将结果存入
void Endpoint::battle(int PokemonID, int enemySpeciesID, int enemyLevel,
                      int autofight) {
  // get player's pokemon
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  string sql;
  sql = "SELECT id, specie, name, atk, def, maxHp, attackgap, lv, exp, pp FROM "
        "Pokemon "
        "where id=" +
        to_string(PokemonID) + ";";
  if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
    return;
  }

  // send result to client
  // 返回格式 specie\nname\n
  string result;
  for (int i = 1; i < 3; ++i) {
    result += sqlResult[10 + i];
    result += "\n";
  }
  result += to_string(enemySpeciesID) + "\n";
  result += "enemy\n";
  strcpy(buf, result.c_str());
  send(connectSocket, buf, BUF_LENGTH, 0);

  // get player's pokemon
  auto playerPokemon = make_shared<components::spirits::Pokemon>(
      static_cast<components::spirits::PokemonBase::Species>(
          stoi(sqlResult[11])),
      sqlResult[12], stoi(sqlResult[13]), stoi(sqlResult[14]),
      stoi(sqlResult[15]), stoi(sqlResult[16]), stoi(sqlResult[17]),
      stoi(sqlResult[18]), stoi(sqlResult[19]));
  // get enemy's pokemon
  auto enemyPokemon = playerPokemon->getEnemy(enemySpeciesID, enemyLevel);
  // battle
  auto Battle = make_unique<BattleController>(playerPokemon, enemyPokemon,
                                              autofight, connectSocket);
  // 如果获胜
  if (Battle->start()) {
    // 如果胜利,胜场数和总场数加1
    string sql_update =
        "update User set win = win + 1, total = total + 1 where id=" +
        to_string(playerID) + ";";
    if (sqlite3_exec(db, sql_update.c_str(), nonUseCallback, NULL, &errMsg) !=
        SQLITE_OK) {
      cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
      cout << "Something wrong happened.\n";
      sqlite3_free(errMsg);
    }
    // 增加100经验值
    playerPokemon->upGrade(100);

    // 如果是决斗赛
    if (isDuel) {
      savePokemonToDB(enemyPokemon);
    }
    savePokemonToDB(playerPokemon, PokemonID);
  } else {
    // 如果失败,总场数加1
    string sql_update =
        "update User set total = total + 1 where id=" + to_string(playerID) +
        ";";
    if (sqlite3_exec(db, sql_update.c_str(), nonUseCallback, NULL, &errMsg) !=
        SQLITE_OK) {
      cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
      cout << "Something wrong happened.\n";
      sqlite3_free(errMsg);
    }
    savePokemonToDB(playerPokemon, PokemonID);

    memset(buf, 0, sizeof(buf));
    // 持续监听得到Lose response
    while (true) {
      int bytesReceived = recv(connectSocket, buf, BUF_LENGTH, 0);
      if (bytesReceived < 0) {
        cout << "Error reading from server." << endl;
        close(connectSocket); // 正确关闭socket
        exit(1);
      } else if (bytesReceived == 0) {
        // 如果recv返回0，
      } else {
        // 正常接收到数据
        auto strs = Helper::split(std::string(buf), '\n');
        if (strs[0] == "Lose response") {
          if (isDuel) {
            string str = "Duel\n" + chooseBet();
            // 将数据传给客户端
            strcpy(buf, str.c_str());
            send(connectSocket, buf, BUF_LENGTH, 0);
          } else {
            strcpy(buf, "Upgrade\n");
            send(connectSocket, buf, BUF_LENGTH, 0);
          }
        } else {
          std::cout << "The server said Lose response error \n";
        }
        memset(buf, 0, sizeof(buf));
        break;
      }
    }
  }
  // 释放表格
  sqlite3_free_table(sqlResult);
}

// 将对应ID的精灵从自己的精灵列表数据库中删除,
// 正常情形不会丢弃他人的精灵，但加以判断加强安全性
void Endpoint::discard(int PokemonID) {
  string sql_check =
      "SELECT userid FROM Pokemon where id = " + to_string(PokemonID) + ";";
  string sql_delete =
      "delete from Pokemon where id = " + to_string(PokemonID) + ";";
  char **sqlResult;
  int nRow;
  int nColumn;
  char *errMsg;
  if (sqlite3_get_table(db, sql_check.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
  } else {
    string searchID = sqlResult[1];
    cout << "DEBUG :: The searched playerID is " << searchID << endl;
    if (searchID != to_string(playerID)) {
      strcpy(buf, "The Pokemon is not belong to you...\n");
      send(connectSocket, buf, BUF_LENGTH, 0);
    } else {
      if (sqlite3_exec(db, sql_delete.c_str(), nonUseCallback, NULL, &errMsg) !=
          SQLITE_OK) {
        cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg
             << endl;
        strcpy(buf, errMsg);
        sqlite3_free(errMsg);
        send(connectSocket, buf, BUF_LENGTH, 0);
      } else {
        strcpy(buf, "Accept.\n");
        send(connectSocket, buf, BUF_LENGTH, 0);
      }
    }
  }
  // 进一步判断用户是否没有精灵了，如果没有了就随机给他一只
  string sql =
      "SELECT id FROM Pokemon where userid=" + to_string(playerID) + ";";
  if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn,
                        &errMsg) != SQLITE_OK) {
    cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
    cout << "TEST DEBUG" << endl;
    sqlite3_free(errMsg);
    strcpy(buf, "Reject: Server error.\n");
    send(connectSocket, buf, BUF_LENGTH, 0);
    return;
  }
  if (nRow == 1) {
    // add pokemons for user
    int Index = rand() % 4;
    auto t = make_shared<components::spirits::Pokemon>(
        static_cast<components::spirits::PokemonBase::Species>(Index),
        Name[Index]);
    cout << "Run out of Pokemon\n Give him one...";
    savePokemonToDB(t);
  }
  sqlite3_free_table(sqlResult);
}
