#ifndef ADD_USER_ITEM_HPP
#define ADD_USER_ITEM_HPP
#include "list_item_base.hpp"

namespace Ui {
class AddUserItem;
}
/**
 * @brief SearchList中的列表项类型，用于添加用户时，表示搜索到的用户的控件，
 *
 */
class AddUserItem : public ListItemBase {
  Q_OBJECT

public:
  explicit AddUserItem(QWidget *parent = nullptr);
  ~AddUserItem();
  QSize sizeHint() const override {
    return QSize(250, 70); // 返回自定义的尺寸
  }

protected:
private:
  Ui::AddUserItem *ui;
};

#endif