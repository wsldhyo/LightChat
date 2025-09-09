#ifndef GROUP_TIP_ITEM_HPP
#define GROUP_TIP_ITEM_HPP
#include "list_item_base.hpp"
namespace Ui {
class GroupTipItem;
};

/**
 * @brief 联系人列表中的组标题条目（例如 “联系人”、“新的朋友” 分组提示）
 *
 * 用于在 QListWidget 中显示分组提示，不可被选中。
 */
class GroupTipItem : public ListItemBase {
  Q_OBJECT

public:
  explicit GroupTipItem(QWidget *parent = nullptr);
  ~GroupTipItem();

  /// 返回自定义的推荐大小
  QSize sizeHint() const override;

  /// 设置分组提示文本
  void SetGroupTip(QString str);

private:
  QString _tip;         ///< 存储分组提示文字
  Ui::GroupTipItem *ui; 
};
#endif // GROUP_TIP_ITEM