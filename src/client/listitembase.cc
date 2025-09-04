#include "listitembase.hpp"

ListItemBase::ListItemBase(QWidget *parent) : QWidget(parent) {}

void ListItemBase::SetItemType(ListItemType item_type) { item_type_ = item_type; }

ListItemType ListItemBase::GetItemType() { return item_type_; }