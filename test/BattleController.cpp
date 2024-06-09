#include "BattleController.hpp"
#include <chrono>
#include <memory>
#include <thread>
using std::cout;
BattleController::BattleController(
    std::shared_ptr<components::spirits::Pokemon> p1,
    std::shared_ptr<components::spirits::Pokemon> p2, bool autoFight)
    : p1(p1), p2(p2), autoFight(autoFight), timer1(0), timer2(0) {}

// 实际上有些属性可以不用被封装
void BattleController::start() {
  cout << p1->GetName() << " VS " << p2->GetName() << "!\n";
  cout << "Battle Start!\n\n";

  // restore all pp and hp and other attributes before battle start
  p1->restoreAll();
  p2->restoreAll();

  timer1 = 0;
  timer2 = 0;

  int damage = 0;
  // cout << "shared count of left has " << p1.use_count() << endl;
  // cout << "shared count of right has " << p2.use_count() << endl;
  // cout << endl;
  while (true) {
    while (timer1 < p1->GetAttackFrequence() &&
           timer2 < p2->GetAttackFrequence()) {
      ++timer1;
      ++timer2;
    }
    // cout << "shared count of left has " << p1.use_count() << endl;
    // cout << "shared count of right has " << p2.use_count() << endl;

    if (timer1 >= p1->GetAttackFrequence() &&
        timer2 >= p2->GetAttackFrequence()) {
      // this case means two pokemon attack at the same time, judge their speed
      if (p1->GetAttackFrequence() <= p2->GetAttackFrequence()) {
        damage = p1->attack(p2, autoFight);
        if (p2 == nullptr) {
          cout << "It become wild!!!!!!!!" << std::endl;
        }
        p2->changeBattleHP(damage);
        if (p2->GetCurHP() <= 0)
          break;
        damage = p2->attack(p1, autoFight);
        p1->changeBattleHP(damage);
        if (p1->GetCurHP() <= 0)
          break;
      } else {
        damage = p2->attack(p1, autoFight);
        p1->changeBattleHP(damage);
        if (p1->GetCurHP() <= 0)
          break;
        damage = p1->attack(p2, autoFight);
        p2->changeBattleHP(damage);
        if (p2->GetCurHP() <= 0)
          break;
      }
      timer1 = timer2 = 0;
    } else if (timer1 >= p1->GetAttackFrequence()) {
      // p1 attack
      damage = p1->attack(p2, autoFight);
      p2->changeBattleHP(damage);
      if (p2->GetCurHP() <= 0)
        break;
      timer1 = 0;
    } else {
      // p2 attack
      damage = p2->attack(p1, autoFight);
      p1->changeBattleHP(damage);
      if (p1->GetCurHP() <= 0)
        break;
      timer2 = 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    //    cout << "First round over!!!" << std::endl;
    // cout << "shared count of left has " << p1.use_count() << endl;
    // cout << "shared count of right has " << p2.use_count() << endl;
    //  break;
    //    p1->debugInfo();
    //  p2->debugInfo();
  }

  // cout << "the game is safely over now!!" << std::endl;
  if (p1->GetCurHP() > 0) {
    cout << p1->GetName() << " won!\n\n";
  } else {
    cout << p2->GetName() << " won!\n\n";
  }
}
