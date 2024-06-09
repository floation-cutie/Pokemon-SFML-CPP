#ifndef POKEMON_HPP
#define POKEMON_HPP

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
namespace components::spirits {
/**
 * @brief implement class of pokemon spirits
 * 在本游戏中全部考虑数值转化为整数
 */

// 经验值列表
const int LEVEL_EXP_LIST[15] = {0,    100,  250,  500,  800,  1200, 1800, 2500,
                                3300, 4500, 6000, 7000, 8000, 9000, 10000};
#define CRITICAL_RATE 0.1 // 暴击概率
#define MISS_RATE 0.2     // 闪避概率
using std::cout;
using std::endl;
// forward declaration for PokemonBase to use
// the pointer to the instantiated Pokemon class
class Pokemon;

//精灵基类
class PokemonBase {
public:
  using Level = unsigned int;
  using EXP = unsigned int;
  using HP = int;
  using attackgap = time_t;

  // 精灵主类别,暂时不用enum class,显示转换的过程在后续实现
  enum Species {
    strength,  // 高攻击
    tank,      // 高生命
    defensive, // 高防御
    agile      // 低攻击间隔
  };

  // 精灵核心属性
  typedef struct coreAtrribute {
    Species specie;
    int attackPoint;
    int defensePoint;
    HP hp;
    attackgap attackGap;
    std::string skillName[2]; //两类进攻方式
    std::string skillDesc[2]; //进攻描述
    int pp;                   //使用次数
  } coreAtrribute;

protected:
  // 统一属性
  Level _level;
  EXP _exp;
  coreAtrribute _coreAtrribute;
  // 唯一id
  size_t _id;

public:
  explicit PokemonBase(Species specie);
  // PokemonBase(const PokemonBase &other) = default;
  //  getter
  Species GetSpecie() const { return _coreAtrribute.specie; }
  Level GetLevel() const { return _level; }
  EXP GetExp() const { return _exp; }
  int GetAttackPoint() const { return _coreAtrribute.attackPoint; }
  int GetHealthPoint() const { return _coreAtrribute.hp; }
  int GetDefensePoint() const { return _coreAtrribute.defensePoint; }
  int GetAttackFrequence() const { return _coreAtrribute.attackGap; }
  int GetId() const { return _id; }
  int GetPowerPoint() const { return _coreAtrribute.pp; }
  std::string GetSkillName(int n) const;
  std::string GetSkillDesc(int n) const;

  //考虑属性之间的碰撞
  virtual int attack(std::shared_ptr<Pokemon> opptr, std::string &msg,
                     bool autofight = true, int manualSkillIndex = 0) = 0;
};

class Pokemon : public Entity,
                public PokemonBase,
                public std::enable_shared_from_this<Pokemon> {

private:
  // 增加随机性
  int randombonus(unsigned int maximum) { return std::rand() % maximum + 1; }
  /**
   * 提供对应的技能点和技能介绍
   */
  void bornWithSkill(Species specie) {
    switch (specie) {
    case Species::agile:
      _coreAtrribute.skillName[0] = std::string("kick");
      _coreAtrribute.skillName[1] = std::string("take down");
      _coreAtrribute.skillDesc[0] = std::string("Simple kick");
      _coreAtrribute.skillDesc[1] =
          std::string("Cause huge damage based on current speed");
      _coreAtrribute.pp = 5;
      break;
    case Species::defensive:
      _coreAtrribute.skillName[0] = std::string("kick");
      _coreAtrribute.skillName[1] = std::string("icon defence");
      _coreAtrribute.skillDesc[0] = std::string("Simple kick");
      _coreAtrribute.skillDesc[1] = std::string("Improve defence");
      _coreAtrribute.pp = 5;
      break;
    case Species::strength:
      _coreAtrribute.skillName[0] = std::string("kick");
      _coreAtrribute.skillName[1] = std::string("rage");
      _coreAtrribute.skillDesc[0] = std::string("Simple kick");
      _coreAtrribute.skillDesc[1] = std::string("Improve strength");
      _coreAtrribute.pp = 7;
      break;
    case Species::tank:
      _coreAtrribute.skillName[0] = std::string("kick");
      _coreAtrribute.skillName[1] = std::string("life drain");
      _coreAtrribute.skillDesc[0] = std::string("Simple kick");
      _coreAtrribute.skillDesc[1] = std::string("Cause damage and restore HP");
      _coreAtrribute.pp = 5;
      break;
    }
  }

public:
  //不提供默认构造函数
  //用于客户端显示
  explicit Pokemon(Species specie, const std::string &name,
                   const TextureHolder &textures);
  //用于初次创建
  explicit Pokemon(Species specie, const std::string &name);
  // 用于服务器对战创建
  explicit Pokemon(Species specie, const std::string &name, int atk, int def,
                   int maxHp, time_t attackgap, int level, int exp, int pp);
  //通过数据库copy创建
  explicit Pokemon(Species specie, const std::string &name, int atk, int def,
                   int maxHp, time_t attackgap, int level, int exp, int pp,
                   const TextureHolder &textures);
  //  Pokemon(const Pokemon &other) = delete;
  virtual ~Pokemon() = default;
  // runtime battle
  virtual int attack(const std::shared_ptr<Pokemon> opptr, std::string &msg,
                     bool autofight = true, int manualSkillIndex = 0) override;

  std::shared_ptr<Pokemon> getsharedptr() { return shared_from_this(); }
  // getter
  void debugInfo() const;
  std::string GetName() const { return name; }
  std::string GetSpeiceInfo() const;
  HP GetCurHP() const { return battleHp; };
  // setter设置器,只考虑向下单调
  void changeBattleHP(int change) { battleHp -= change; }
  void changeBattleAttackPoint(int change) { battleAttackPoint += change; }
  void changeBattleDefensePoint(int change) { battleDefensePoint += change; }
  /**@param exp 提供待增加的经验值
   * @brief 判断是否已经升级
   */
  bool upGrade(EXP exp);
  //战斗
  std::shared_ptr<Pokemon> getEnemy(int specieIndex, int lv);
  int attack(std::shared_ptr<Pokemon> one, std::shared_ptr<Pokemon> other,
             int skillIndex, std::string &msg);

  bool judgeMiss(attackgap lhs, attackgap rhs) const;
  bool judgeCritical() const;
  //实际伤害
  int actualHurt(std::shared_ptr<Pokemon> one, std::shared_ptr<Pokemon> other,
                 int damage, std::string &msg);
  void restoreAll();

  // 绘制
  virtual void drawCurrent(sf::RenderTarget &target,
                           sf::RenderStates states) const override;

protected:
  std::string name;
  // 战斗属性
  HP battleHp;
  int battlePP;           //战斗蓝条
  int battleAttackPoint;  //战斗攻击
  int battleDefensePoint; //战斗防御

  sf::Sprite mSprite;
};

inline bool components::spirits::Pokemon::upGrade(EXP exp) {
  if (_level == 15)
    return false;

  _exp += exp;

  cout << GetName() << " gains " << exp << " exp!\n";
  cout << "Now " << GetName() << " has " << _exp << " exp\n" << endl;

  bool LV_UP = false;
  while (_level < 15 && _exp >= LEVEL_EXP_LIST[_level]) {
    // level-up!
    LV_UP = true;
    ++_level;
    cout << "Level Up!\n";
    cout << GetName() << "'s level is " << _level << "!\n" << endl;

    // increase attributes
    int atk, def, maxHp, speed;
    atk = 4 + randombonus(2);
    def = 2 + randombonus(2);
    maxHp = 8 + randombonus(3);
    speed = randombonus(2);

    // race talent
    switch (GetSpecie()) {
    case Species::agile:
      speed += 1;
      break;
    case Species::defensive:
      def += 2;
      break;
    case Species::strength:
      atk += 3;
      break;
    case Species::tank:
      maxHp += 4;
      break;
    }

    _coreAtrribute.attackPoint += atk;
    _coreAtrribute.defensePoint += def;
    _coreAtrribute.hp += maxHp;
    _coreAtrribute.attackGap -= speed;

    cout << "Atk: " << _coreAtrribute.attackPoint - atk << "->"
         << _coreAtrribute.attackPoint << "!\n";
    cout << "Def: " << _coreAtrribute.defensePoint - def << "->"
         << _coreAtrribute.defensePoint << "!\n";
    cout << "MaxHP: " << _coreAtrribute.hp - maxHp << "->" << _coreAtrribute.hp
         << "!\n";
    cout << "Attack Gap: " << _coreAtrribute.attackGap + speed << "->"
         << _coreAtrribute.attackGap << "!\n\n";
  }

  if (LV_UP)
    return true;

  return false; // default
}

} // namespace components::spirits

#endif // !POKEMON_H
