#ifndef LISTITEM_BASE_HPP
#define LISTITEM_BASE_HPP
#include <QWidget>

class ListItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget *parent = nullptr);
    void SetItemType(ListItemType itemType);

    ListItemType GetItemType();
protected:
    void paintEvent(QPaintEvent* event);
private:
    ListItemType _itemType;

public slots:

signals:


};

#endif // LISTITEMBASE_H
