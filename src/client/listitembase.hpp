#ifndef LISTITEMBASE_HPP
#define LISTITEMBASE_HPP
#include "client_constant.hpp"
#include <QWidget>
/**
 * @brief 列表项的基类
 *
 */
class ListItemBase : public QWidget {
  Q_OBJECT
public:
  explicit ListItemBase(QWidget *parent = nullptr);
  void SetItemType(ListItemType item_type);

  ListItemType GetItemType();

private:
  ListItemType item_type_;

public slots:

signals:
};

#endif // LISTITEMBASE_H