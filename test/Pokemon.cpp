#include "Pokemon.hpp"
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

int components::spirits::Pokemon::attack(std::shared_ptr<Pokemon> opptr,
                                         bool autofight) {

  int skillIndex = 0;
  if (autofight) {
    //五级之后有特殊能力
    int usableCount = 1; // can use simple attack by default
    if (_level >= 5) {
      ++usableCount;
    }
    // get a random skill 0 or 1
    skillIndex = rand() % usableCount;
    // find the skill
    if (skillIndex > 0) {
      if (battlePP > 0) {
        battlePP--; //
      } else {
        skillIndex = 0;
      }
    }

    // overloaded function
    return attack(this->getsharedptr(), opptr, skillIndex);
    // auto thisShared = this->getsharedptr();
    //  cout << "\nshared count of left has " << thisShared.use_count() << endl;
    //  int dk = attack(thisShared, opptr, skillIndex);
    // cout << "\nshared count of left has " << thisShared.use_count() << endl;
    // cout << "shared count of right has " << opptr.use_count() << endl;
    // cout << "DEBUG: Cause " << dk << "\n";
    //    return dk;
  } else {
    return 0;
  }
}
// agile --> tank --> strength --> defensive -->
int components::spirits::Pokemon::attack(std::shared_ptr<Pokemon> one,
                                         std::shared_ptr<Pokemon> other,
                                         int skillIndex) {
  // cout << endl;
  // cout << "shared count of left has " << one.use_count() << endl;
  // cout << "shared count of right has " << other.use_count() << endl;

  cout << one->GetName() << " uses "
       << one->_coreAtrribute.skillName[skillIndex] << " to "
       << other->GetName() << "!\n";
  auto coefficient = 1.0;
  switch (one->GetSpecie()) {
  case Species::tank: {
    if (other->GetSpecie() == Species::strength) {
      coefficient = 1.3;
    }
    switch (skillIndex) {
    case 0: // kick
      return static_cast<int>(coefficient *
                              actualHurt(one, other, one->battleAttackPoint));
    case 1: // life drain
      int attack = static_cast<int>(
          0.7 * coefficient * actualHurt(one, other, one->battleAttackPoint));
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
      return static_cast<int>(coefficient *
                              actualHurt(one, other, one->battleAttackPoint));
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
      return static_cast<int>(coefficient *
                              actualHurt(one, other, one->battleAttackPoint));
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
      return static_cast<int>(coefficient *
                              actualHurt(one, other, one->battleAttackPoint));
    case 1: // take down
      int baseAttackGap = 100;
      return static_cast<int>(coefficient *
                              actualHurt(one, other, one->battleAttackPoint) *
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
                                             int damage) {
  if (judgeMiss(one->GetAttackFrequence(), other->GetAttackFrequence())) {
    cout << "This round missed from " << this->GetName() << " !\n";
    return 0;
  } else if (judgeCritical()) {
    cout << "This round has critical attack from " << this->GetName() << " !\n";
    int actualHurt = static_cast<int>(1.5 * damage) - other->GetDefensePoint();
    if (actualHurt > 0) {
      return actualHurt;
    }
    return 0;
  } else {
    cout << "This round has normal attack from " << this->GetName() << " !\n";
    // cout << "DEBUG:: damage is " << damage << endl;
    // cout << "DEBUG:: defence is " << other->GetDefensePoint() << endl;
    int actualHurt = damage - other->GetDefensePoint();
    // cout << "DEBUG:: actualHurt is " << actualHurt << endl;
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
