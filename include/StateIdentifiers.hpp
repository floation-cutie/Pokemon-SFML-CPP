#ifndef BOOK_STATEIDENTIFIERS_HPP
#define BOOK_STATEIDENTIFIERS_HPP

namespace States {
// 标题, 菜单, 加载,暂停... (登录界面)
enum ID {
  None,
  Title,
  Menu,
  Game,
  Loading,
  Pause,
  Battle,
  ChooseBattle,
  Settings,
};
} // namespace States

#endif // BOOK_STATEIDENTIFIERS_HPP
