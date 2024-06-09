#include "../include/Pokemon.hpp"
#include "../include/ResourceHolder.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

// to generate a random number for expansion
#include <cstdlib>
#include <ctime>

using namespace components::spirits;

using std::cout;
using std::endl;
//考虑初始化列表方式
components::spirits::PokemonBase::PokemonBase(Species specie) {
  _coreAtrribute.specie = specie;
  _coreAtrribute.attackPoint = 10;
  _coreAtrribute.defensePoint = 5;
  _coreAtrribute.hp = 50;
  _coreAtrribute.attackGap = 100; // ms

  switch (specie) {
  case Species::tank:
    _coreAtrribute.hp += 10;
    break;
  case Species::agile:
    _coreAtrribute.attackGap -= 4;
  case Species::defensive:
    _coreAtrribute.defensePoint += 3;
    break;
  case Species::strength:
    _coreAtrribute.attackPoint += 3;
    break;
  }
}

std::string components::spirits::PokemonBase::GetSkillName(int n) const {
  if (n >= 0 && n < 2) {
    return _coreAtrribute.skillName[n];
  }
  return "The index for skill name is invalid";
}

std::string components::spirits::PokemonBase::GetSkillDesc(int n) const {
  if (n >= 0 && n < 2) {
    return _coreAtrribute.skillDesc[n];
  }
  return "The index for skill description is invalid";
}

Textures::ID toTextureID(components::spirits::PokemonBase::Species specie) {
  switch (specie) {
  case PokemonBase::Species::strength:
    return Textures::strength;
  case PokemonBase::Species::defensive:
    return Textures::defensive;
  case PokemonBase::Species::agile:
    return Textures::agile;
  case PokemonBase::Species::tank:
    return Textures::tank;
  }
  return Textures::tank;
}

std::string components::spirits::Pokemon::GetSpeiceInfo() const {
  switch (PokemonBase::GetSpecie()) {
  case Species::agile:
    return "High speed";
  case Species::defensive:
    return "High defence";
  case Species::strength:
    return "High attack";
  case Species::tank:
    return "High HP";
  }
  return "";
}

void components::spirits::Pokemon::debugInfo() const {
  // debug info
  cout << "Name: " << GetName() << endl
       << "Specie: " << GetSpeiceInfo() << endl
       << "Attack: " << GetAttackPoint() << endl
       << "Def: " << GetDefensePoint() << endl
       << "MaxHp: " << GetHealthPoint() << endl
       << "Attack Gap: " << GetAttackFrequence() << endl
       << "Level: " << GetLevel() << endl
       << "Exp: " << GetExp() << endl;

  // output skill
  cout << "Skills:\n";
  for (int i = 0; i < 2; ++i) {
    cout << "	Name: " << GetSkillName(i) << endl;
    cout << "	Description: " << GetSkillDesc(i) << endl;
    if (i) {
      cout << "	PP: " << GetPowerPoint() << endl;
    } else {
      cout << "	PP: infinity\n";
    }
  }
  cout << endl;
}
components::spirits::Pokemon::Pokemon(Species specie, const std::string &name)
    : PokemonBase(specie) {
  srand(time(NULL));
  if (name.length() == 0) {
    std::cerr << "Should specify a particular name for your spirit\n";
    exit(1);
  } else {
    this->name = name;
  }

  bornWithSkill(specie);
  // add some random factor
  _coreAtrribute.attackPoint += randombonus(3);
  _coreAtrribute.defensePoint += randombonus(3);
  _coreAtrribute.attackGap -= randombonus(3);
  _coreAtrribute.hp += randombonus(4);
  _coreAtrribute.pp += randombonus(3);
  _level = 1;
  _exp = 0;

  debugInfo();
}

components::spirits::Pokemon::Pokemon(Species specie, const std::string &name,
                                      const TextureHolder &textures)
    : PokemonBase(specie), mSprite(textures.get(toTextureID(specie))) {
  srand(time(NULL));
  //表示其局部布局,对于非碰撞体来说影响不大
  sf::FloatRect bounds = mSprite.getLocalBounds();
  mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

  if (name.length() == 0) {
    std::cerr << "Should specify a particular name for your spirit\n";
    exit(1);
  } else {
    this->name = name;
  }

  bornWithSkill(specie);
  // add some random factor
  _coreAtrribute.attackPoint += randombonus(3);
  _coreAtrribute.defensePoint += randombonus(3);
  _coreAtrribute.attackGap -= randombonus(3);
  _coreAtrribute.hp += randombonus(4);
  _coreAtrribute.pp += randombonus(3);
  _level = 1;
  _exp = 0;

  // debugInfo();
}
//用于服务器对战创建
components::spirits::Pokemon::Pokemon(Species specie, const std::string &name,
                                      int atk, int def, int MaxHp,
                                      time_t attackgap, int level, int exp,
                                      int pp)
    : PokemonBase(specie), name(name) {
  srand(time(NULL));
  bornWithSkill(specie);
  _coreAtrribute.pp = pp;
  _coreAtrribute.hp = MaxHp;
  _coreAtrribute.attackGap = attackgap;
  _coreAtrribute.attackPoint = atk;
  _coreAtrribute.defensePoint = def;
  _level = level;
  _exp = exp;

  debugInfo();
}

components::spirits::Pokemon::Pokemon(Species specie, const std::string &name,
                                      int atk, int def, int MaxHp,
                                      time_t attackgap, int level, int exp,
                                      int pp, const TextureHolder &textures)
    : PokemonBase(specie), name(std::move(name)),
      mSprite(textures.get(toTextureID(specie))) {
  srand(time(NULL));
  sf::FloatRect bounds = mSprite.getLocalBounds();
  mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  bornWithSkill(specie);
  _coreAtrribute.pp = pp;
  _coreAtrribute.hp = MaxHp;
  _coreAtrribute.attackGap = attackgap;
  _coreAtrribute.attackPoint = atk;
  _coreAtrribute.defensePoint = def;
  _level = level;
  _exp = exp;

  debugInfo();
}

// 在函数中创建一个对于对手Pokemon的unique_ptr，返回方式以shared_ptr的方式返回
std::shared_ptr<Pokemon> components::spirits::Pokemon::getEnemy(int specieIndex,
                                                                int lv) {
  if (specieIndex < 0 || specieIndex > 3) {
    std::cerr << "Invalid specie index\n";
    exit(1);
  }
  if (lv < 1 || lv > 15) {
    std::cerr << "Invalid level\n";
    exit(1);
  }
  Species specie = static_cast<Species>(specieIndex);
  std::string name = "Enemy";
  std::shared_ptr<Pokemon> enemy(new Pokemon(specie, name));
  enemy->upGrade(LEVEL_EXP_LIST[lv - 1]);
  return enemy;
}

void components::spirits::Pokemon::drawCurrent(sf::RenderTarget &target,
                                               sf::RenderStates states) const {
  target.draw(mSprite, states);
}

int components::spirits::Pokemon::attack(std::shared_ptr<Pokemon> opptr,
                                         std::string &msg, bool autofight,
                                         int ManualSkillIndex) {
  msg = "";
  int skillIndex = 0;
  // TODO
  //五级之后有特殊能力
  int usableCount = 2; // can use simple attack by default
  /*
  if (_level >= 5) {
    ++usableCount;
  }
  */
  if (autofight) {

    // get a random skill 0 or 1
    skillIndex = rand() % usableCount;
    // find the skill
    if (skillIndex > 0) {
      if (battlePP > 0) {
        battlePP--;
      } else {
        skillIndex = 0;
      }
    }

    // overloaded function
    return attack(this->getsharedptr(), opptr, skillIndex, msg);
    // auto thisShared = this->getsharedptr();
    //  cout << "\nshared count of left has " << thisShared.use_count() << endl;
  } else {
    // manual fight, get skillIndex

    // 当蓝条为0时，返回该攻击蓝条消耗完毕的消息给客户端
    msg = GetName() + ", your turn!\n";
    msg += "Choose a skill to attack!\n";
    // 格式化输出技能和技能描述以及pp值
    for (int i = 0; i < usableCount; ++i) {
      msg += std::to_string(i + 1) + ". " + _coreAtrribute.skillName[i] + "\n";
      msg += "  " + _coreAtrribute.skillDesc[i] + "\n";
      if (i) {
        msg += "  PP: " + std::to_string(battlePP) + "\n";
      } else {
        msg += "  PP: infinity\n";
      }
    }
    if (ManualSkillIndex > 0) {
      if (battlePP > 0) {
        battlePP--;
      } else {
        msg += "PP is not enough, use simple attack instead\n";
        ManualSkillIndex = 0;
      }
    }
    msg += "It use skill " + _coreAtrribute.skillName[ManualSkillIndex] + "\n";
    return attack(this->getsharedptr(), opptr, ManualSkillIndex, msg);
  }
}
// agile --> tank --> strength --> defensive -->
int components::spirits::Pokemon::attack(std::shared_ptr<Pokemon> one,
                                         std::shared_ptr<Pokemon> other,
                                         int skillIndex, std::string &msg) {
  // cout << endl;
  // cout << "shared count of left has " << one.use_count() << endl;
  // cout << "shared count of right has " << other.use_count() << endl;
  auto coefficient = 1.0;
  switch (one->GetSpecie()) {
  case Species::tank: {
    if (other->GetSpecie() == Species::strength) {
      coefficient = 1.3;
    }
    switch (skillIndex) {
    case 0: // kick
      return static_cast<int>(
          coefficient * actualHurt(one, other, one->battleAttackPoint, msg));
    case 1: // life drain
      int attack =
          static_cast<int>(0.7 * coefficient *
                           actualHurt(one, other, one->battleAttackPoint, msg));
      if (static_cast<int>(one->GetHealthPoint() - one->battleHp) <=
          static_cast<int>(attack * 0.5))
        one->battleHp = one->GetHealthPoint();
      else
        one->battleHp += static_cast<int>(attack * 0.5);
      return attack;
      break;
    }
  } break;
  case Species::defensive: {
    if (other->GetSpecie() == Species::agile) {
      coefficient = 1.3;
    }
    switch (skillIndex) {
    case 0: // kick
      return static_cast<int>(
          coefficient * actualHurt(one, other, one->battleAttackPoint, msg));
    case 1: // icon defence
      one->battleDefensePoint = static_cast<int>(one->battleDefensePoint * 1.1);
      return 0;
      break;
    }
  } break;

  case Species::strength: {
    if (other->GetSpecie() == Species::defensive) {
      coefficient = 1.3;
    }
    switch (skillIndex) {
    case 0: // kick
      return static_cast<int>(
          coefficient * actualHurt(one, other, one->battleAttackPoint, msg));
    case 1: // rage
      one->battleAttackPoint = static_cast<int>(one->battleAttackPoint * 1.25);
      return 0;
      break;
    }
  } break;

  case Species::agile: {
    if (other->GetSpecie() == Species::strength) {
      coefficient = 1.3;
    }
    switch (skillIndex) {
    case 0: // kick
      return static_cast<int>(
          coefficient * actualHurt(one, other, one->battleAttackPoint, msg));
    case 1: // take down
      int baseAttackGap = 100;
      return static_cast<int>(
          coefficient * actualHurt(one, other, one->battleAttackPoint, msg) *
          baseAttackGap / one->GetAttackFrequence());
      break;
    }
  }
  }

  return 0;
}

bool components::spirits::Pokemon::judgeCritical() const {
  if (rand() % 100 / 100.0 <= CRITICAL_RATE) {
    return true;
  }
  return false;
}

bool components::spirits::Pokemon::judgeMiss(attackgap lhs,
                                             attackgap rhs) const {
  if ((lhs - rhs + rand() % 7) / 100.0 >= MISS_RATE) {
    return true;
  }
  return false;
}

int components::spirits::Pokemon::actualHurt(std::shared_ptr<Pokemon> one,
                                             std::shared_ptr<Pokemon> other,
                                             int damage, std::string &msg) {
  if (judgeMiss(one->GetAttackFrequence(), other->GetAttackFrequence())) {
    msg += "This round missed from " + one->GetName() + " !\n";
    return 0;
  } else if (judgeCritical()) {
    msg += "This round has critical attack from " + one->GetName() + " !\n";
    int actualHurt = static_cast<int>(1.5 * damage) - other->GetDefensePoint();
    if (actualHurt > 0) {
      return actualHurt;
    }
    return 0;
  } else {
    msg += "This round has normal attack from " + one->GetName() + " !\n";
    int actualHurt = damage - other->GetDefensePoint();
    if (actualHurt > 0) {
      return actualHurt;
    }
    return 0;
  }
}

//战斗前属性
void components::spirits::Pokemon::restoreAll() {
  battleHp = _coreAtrribute.hp;
  battlePP = _coreAtrribute.pp;
  battleAttackPoint = _coreAtrribute.attackPoint;
  battleDefensePoint = _coreAtrribute.defensePoint;
}
