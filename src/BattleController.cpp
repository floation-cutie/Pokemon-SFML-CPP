#include "../include/BattleController.hpp"
#include "../include/HelperClass.hpp"
#include "../include/NetworkProtocol.hpp"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
using std::cout;

// int round = 1;
BattleController::BattleController(
    std::shared_ptr<components::spirits::Pokemon> p1,
    std::shared_ptr<components::spirits::Pokemon> p2, bool autoFight,
    int connectSock)
    : p1(p1), p2(p2), autoFight(autoFight), timer1(0), timer2(0),
      connectSock(connectSock), msg("") {}

void BattleController::autoFightFunc() {
  int damage = 0;
  //对战开始前,接收玩家准备对战的信息
  if (recv(connectSock, recvBuf, BUF_LENGTH, 0) < 0) {
    std::cerr << "Error reading from server." << std::endl;
    close(connectSock); // 正确关闭socket
    exit(1);
  }
  auto strs = Helper::split(recvBuf, '\n');
  memset(recvBuf, 0, BUF_LENGTH);
  if (strs[0] != "attack") {
    std::cout << "ERROR!!!\n";
  }
  while (true) {
    // ----------  对战逻辑 --------------
    while (timer1 < p1->GetAttackFrequence() &&
           timer2 < p2->GetAttackFrequence()) {
      ++timer1;
      ++timer2;
    }

    if (timer1 >= p1->GetAttackFrequence() &&
        timer2 >= p2->GetAttackFrequence()) {
      // this case means two pokemon attack at the same time, judge their
      // speed
      if (p1->GetAttackFrequence() <= p2->GetAttackFrequence()) {
        damage = p1->attack(p2, msg, autoFight);
        p2->changeBattleHP(damage);
        if (p2->GetCurHP() <= 0)
          break;
        damage = p2->attack(p1, msg, true);
        p1->changeBattleHP(damage);
        if (p1->GetCurHP() <= 0)
          break;
      } else {
        damage = p2->attack(p1, msg, true);
        p1->changeBattleHP(damage);
        if (p1->GetCurHP() <= 0)
          break;
        damage = p1->attack(p2, msg, autoFight);
        p2->changeBattleHP(damage);
        if (p2->GetCurHP() <= 0)
          break;
      }
      timer1 = timer2 = 0;
    } else if (timer1 >= p1->GetAttackFrequence()) {
      // p1 attack
      damage = p1->attack(p2, msg, autoFight);
      p2->changeBattleHP(damage);
      if (p2->GetCurHP() <= 0)
        break;
      timer1 = 0;
    } else {
      // p2 attack
      damage = p2->attack(p1, msg, true);
      p1->changeBattleHP(damage);
      if (p1->GetCurHP() <= 0)
        break;
      timer2 = 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
  }
}
void BattleController::manualFightFunc() {
  // autoFight == false
  while (true) {
    int damage;
    int skillIndex = 0;

    //双方当前血量
    // std::cout << "Round " << round++ << std::endl;
    // std::cout << p1->GetName() << " HP: " << p1->GetCurHP() << std::endl;
    //    std::cout << p2->GetName() << " HP: " << p2->GetCurHP() << std::endl;
    // ----------  对战逻辑 --------------
    while (timer1 < p1->GetAttackFrequence() &&
           timer2 < p2->GetAttackFrequence()) {
      ++timer1;
      ++timer2;
    }

    if (timer1 >= p1->GetAttackFrequence() &&
        timer2 >= p2->GetAttackFrequence()) {
      // this case means two pokemon attack at the same time, judge their
      // speed
      if (p1->GetAttackFrequence() <= p2->GetAttackFrequence()) {
        std::cout << "Player turn\n";
        //接收客户端消息 格式为 attack\n(1/0)\n
        // 1 0 表示技能序号
        if (recv(connectSock, recvBuf, BUF_LENGTH, 0) < 0) {
          std::cerr << "Error reading from server." << std::endl;
          close(connectSock); // 正确关闭socket
          exit(1);
        }
        auto strs = Helper::split(recvBuf, '\n');
        memset(recvBuf, 0, BUF_LENGTH);
        if (strs[0] == "attack") {
          skillIndex = std::stoi(strs[1]);
        } else {
          std::cerr << "Error message from client." << std::endl;
          close(connectSock);
          exit(1);
        }
        damage = p1->attack(p2, msg, autoFight, skillIndex);
        p2->changeBattleHP(damage);
        if (p2->GetCurHP() <= 0)
          break;
        // 将msg发送给客户端
        if (send(connectSock, msg.c_str(), msg.size(), 0) < 0) {
          std::cerr << "Error writing to server." << std::endl;
          close(connectSock); // 正确关闭socket
          exit(1);
        }

        damage = p2->attack(p1, msg, true);
        std::cout << msg << std::endl;
        p1->changeBattleHP(damage);
        if (p1->GetCurHP() <= 0)
          break;
      } else {
        damage = p2->attack(p1, msg, true);
        std::cout << msg << std::endl;
        p1->changeBattleHP(damage);
        if (p1->GetCurHP() <= 0)
          break;
        //接收客户端消息 格式为 attack\n(1/0)\n
        // 1 0 表示技能序号
        std::cout << "Player turn\n";
        if (recv(connectSock, recvBuf, BUF_LENGTH, 0) < 0) {
          std::cerr << "Error reading from server." << std::endl;
          close(connectSock); // 正确关闭socket
          exit(1);
        }
        auto strs = Helper::split(recvBuf, '\n');
        memset(recvBuf, 0, BUF_LENGTH);
        if (strs[0] == "attack") {
          skillIndex = std::stoi(strs[1]);
        } else {
          std::cerr << "Error message from client." << std::endl;
          close(connectSock);
          exit(1);
        }
        damage = p1->attack(p2, msg, autoFight, skillIndex);
        p2->changeBattleHP(damage);
        if (p2->GetCurHP() <= 0)
          break;
        // 将msg发送给客户端
        if (send(connectSock, msg.c_str(), msg.size(), 0) < 0) {
          std::cerr << "Error writing to server." << std::endl;
          close(connectSock); // 正确关闭socket
          exit(1);
        }
      }
      timer1 = timer2 = 0;
    } else if (timer1 >= p1->GetAttackFrequence()) {
      //接收客户端消息 格式为 attack\n(1/0)\n
      // 1 0 表示技能序号
      std::cout << "Player turn\n";
      if (recv(connectSock, recvBuf, BUF_LENGTH, 0) < 0) {
        std::cerr << "Error reading from server." << std::endl;
        close(connectSock); // 正确关闭socket
        exit(1);
      }
      auto strs = Helper::split(recvBuf, '\n');
      memset(recvBuf, 0, BUF_LENGTH);
      if (strs[0] == "attack") {
        skillIndex = std::stoi(strs[1]);
      } else {
        std::cerr << "Error message from client." << std::endl;
        close(connectSock);
        exit(1);
      }
      // p1 attack
      damage = p1->attack(p2, msg, autoFight, skillIndex);
      p2->changeBattleHP(damage);
      if (p2->GetCurHP() <= 0)
        break;
      // 将msg发送给客户端
      if (send(connectSock, msg.c_str(), msg.size(), 0) < 0) {
        std::cerr << "Error writing to server." << std::endl;
        close(connectSock); // 正确关闭socket
        exit(1);
      }

      timer1 = 0;
    } else {
      // p2 attack
      damage = p2->attack(p1, msg, true);
      std::cout << msg << std::endl;
      p1->changeBattleHP(damage);
      if (p1->GetCurHP() <= 0)
        break;
      timer2 = 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    // std::cout << "Every round is over!\n\n";
    //  反馈消息
  }
}
// 实际上有些属性可以不用被封装
bool BattleController::start() {
  cout << p1->GetName() << " VS " << p2->GetName() << "!\n";
  cout << "Battle Start!\n\n";

  // restore all pp and hp and other attributes before battle start
  p1->restoreAll();
  p2->restoreAll();
  memset(recvBuf, 0, BUF_LENGTH);
  timer1 = p1->GetAttackFrequence();
  timer2 = 0;
  // cout << endl;
  if (autoFight) {
    autoFightFunc();
  } else {
    manualFightFunc();
  }
  if (p1->GetCurHP() > 0) {
    //将胜利信息传递给客户端
    msg = "Win\n";
    if (send(connectSock, msg.c_str(), msg.size(), 0) < 0) {
      std::cerr << "Error writing to server." << std::endl;
      close(connectSock); // 正确关闭socket
      exit(1);
    }

    return true;
  }
  // 玩家还不知道自己输了,他尝试继续攻击
  else {
    // 服务器一直等待消息到达
    while (true) {
      // 使用recv函数从套接字接收数据
      int bytesReceived = recv(connectSock, recvBuf, BUF_LENGTH, 0);
      // std::cout << "Waiting." << std::endl;
      if (bytesReceived < 0) {
        std::cerr << "Error reading from server." << std::endl;
        close(connectSock);
        exit(1);
      } else if (bytesReceived == 0) {
        // 如果recv返回0，
        // std::cout << "Waiting." << std::endl;
      } else {
        // 正常接收到数据
        auto strs = Helper::split(std::string(recvBuf), '\n');
        if (strs[0] == "attack") {
          std::cout
              << "THE PLayer has lose, but he knows he lose only when he try "
                 "to attack one more time\n";
          msg = "Lose\n";
          if (send(connectSock, msg.c_str(), msg.size(), 0) < 0) {
            std::cerr << "Error writing to server." << std::endl;
            close(connectSock); // 正确关闭socket
            exit(1);
          }
        }
        memset(recvBuf, 0, BUF_LENGTH);
        break;
      }
    }
  }
  return false;
}
