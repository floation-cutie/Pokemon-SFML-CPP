#pragma once
#include "NetworkProtocol.hpp"
#include "Pokemon.hpp"
#include <condition_variable>
#include <memory>
#include <sqlite3.h>
#include <string>
//前向声明
using namespace std;
class GameServer;

class Endpoint {
  // about server
  GameServer &server;

  // about database
  sqlite3 *db;

  // about network
  int port;
  string ip;
  int endpointSocket;
  int connectSocket;
  bool running;
  bool online;
  char buf[BUF_LENGTH];

  // about player
  bool isDuel;
  int playerID;
  string playerUsername;
  size_t pokemonNumIndex;
  size_t pokemonLevelIndex;
  string Honor[3] = {"rookie", "veteran", "master"};
  // about pokemon
  string Name[4] = {"Strengthy", "tanky", "defensive", "agile"};
  // about thread
  mutex mtx;
  condition_variable cv;
  bool timing; // thread function timer is running

  // thread function
  void timerFunc();
  void listenFunc();

  // interface function
  void resetPassword(const string &oldPassword, const string &newPassword);
  void getPlayerList();
  void getPokemonList(int playerID);
  void getPokemonByID(int pokemonID);
  void pokemonChangeName(const string &pokemonID, const string &newName);
  // void useSkill(int skillID);
  void battle(int PokemonID, int enemySpeciesID, int enemyLevel, int autofight);
  string chooseBet();
  void discard(int pokemonID);
  void getHonor();
  // other functions
  // 新建或者更新
  void savePokemonToDB(shared_ptr<components::spirits::Pokemon> p, int id = -1);

public:
  Endpoint(int playerID, sqlite3 *db, GameServer &server);
  ~Endpoint();

  int start();    // return port, return 0 if ERROR
  void process(); // return to delete this endpoint

  bool isOnline() const { return online; }
  int getPlayerID() const { return playerID; }
  string getPlayerName() const { return playerUsername; }
  int getPort() const { return port; }
};
