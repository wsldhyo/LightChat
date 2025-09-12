#include "chat_page.hpp"
#include "chat_item_base.hpp"
#include "client_struct_def.hpp"
#include "msg_text_edit.hpp"
#include "picture_bubble.hpp"
#include "text_bubble.hpp"
#include "ui_chatpage.h"
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
ChatPage::ChatPage(QWidget *parent) : QWidget(parent), ui(new Ui::ChatPage) {
  ui->setupUi(this);
  //设置按钮样式
  ui->receive_btn->SetState("normal", "hover", "press");
  ui->send_btn->SetState("normal", "hover", "press");

  //设置图标样式
  ui->emo_lb->set_state("normal", "hover", "press", "normal", "hover", "press");
  ui->file_lb->set_state("normal", "hover", "press", "normal", "hover",
                         "press");
  connect(ui->chatEdit, &MessageTextEdit::send, this,
          &ChatPage::on_send_btn_clicked);
}

ChatPage::~ChatPage() { delete ui; }

void ChatPage::paintEvent(QPaintEvent *event) {
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked() {
  auto pTextEdit = ui->chatEdit;
  ChatRole role = ChatRole::SELF;
  QString userName = QStringLiteral("wsldhyo");
  QString userIcon = ":/icons/head_1.jpg";

  const QVector<MsgInfo> &msgList = pTextEdit->getMsgList();
  for (int i = 0; i < msgList.size(); ++i) {
    QString type = msgList[i].msgFlag;
    ChatItemBase *pChatItem = new ChatItemBase(role);
    pChatItem->setUserName(userName);
    pChatItem->setUserIcon(QPixmap(userIcon));
    QWidget *pBubble = nullptr;
    if (type == "text") {

      pBubble = new TextBubble(role, msgList[i].content);
      qDebug() << "bubble width" << pBubble->width() << "chat view width:" << ui->chat_data_list->width();
      if(pBubble->width() < pChatItem->name_width()){
        pChatItem->set_spacer_width(pChatItem->name_width() - pBubble->width());
      }
    } else if (type == "image") {
      pBubble = new PictureBubble(QPixmap(msgList[i].content), role);
    } else if (type == "file") {
    }
    if (pBubble != nullptr) {
      pChatItem->setWidget(pBubble);
      ui->chat_data_list->appendChatItem(pChatItem);
    }
  }
}
