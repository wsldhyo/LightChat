#include "chat_page.hpp"
#include "chat_item_base.hpp"
#include "client_struct_def.hpp"
#include "msg_text_edit.hpp"
#include "picture_bubble.hpp"
#include "tcp_manager.hpp"
#include "text_bubble.hpp"
#include "ui_chatpage.h"
#include "user_data.hpp"
#include "usermgr.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QPainter>
#include <QStyleOption>
#include <QUuid>
ChatPage::ChatPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatPage), user_info_(nullptr) {
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

void ChatPage::set_user_info(std::shared_ptr<UserInfo> user_info) {
  user_info_ = user_info;
  //设置ui界面
  ui->title_lb->setText(user_info_->_name);
  // 先移除界面中的所有聊天会话中的聊天记录条目
  ui->chat_data_list->removeAllItem();
  // 再逐条添加与该好友的聊天记录条目，TODO，只添加视图内的条目
  for (auto &msg : user_info->_chat_msgs) {
    append_chat_msg(msg);
  }
}

void ChatPage::append_chat_msg(std::shared_ptr<TextChatData> msg) {
  auto self_info = UserMgr::getinstance()->get_user_info();
  ChatRole role;
  // todo... 添加聊天显示
  if (msg->_from_uid == self_info->_uid) {
    role = ChatRole::SELF;
    ChatItemBase *pChatItem = new ChatItemBase(role);

    pChatItem->setUserName(self_info->_name);
    pChatItem->setUserIcon(QPixmap(self_info->_icon));
    QWidget *pBubble = nullptr;
    pBubble = new TextBubble(role, msg->_msg_content);
    pChatItem->setWidget(pBubble);
    ui->chat_data_list->appendChatItem(pChatItem);
  } else {
    role = ChatRole::OTHERS;
    ChatItemBase *pChatItem = new ChatItemBase(role);
    auto friend_info =
        UserMgr::getinstance()->get_friend_infO_by_id(msg->_from_uid);
    if (friend_info == nullptr) {
      return;
    }
    pChatItem->setUserName(friend_info->_name);
    pChatItem->setUserIcon(QPixmap(friend_info->_icon));
    QWidget *pBubble = nullptr;
    pBubble = new TextBubble(role, msg->_msg_content);
    pChatItem->setWidget(pBubble);
    ui->chat_data_list->appendChatItem(pChatItem);
  }
}

void ChatPage::paintEvent(QPaintEvent *event) {
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked() {
  if (user_info_ == nullptr) {
    qDebug() << "friend_info is empty";
    return;
  }

  auto self_info = UserMgr::getinstance()->get_user_info();
  auto pTextEdit = ui->chatEdit;
  ChatRole role = ChatRole::SELF;
  QString userName = self_info->_name;
  QString userIcon = self_info->_icon;

  const QVector<MsgInfo> &msgList = pTextEdit->getMsgList();
  QJsonObject textObj;
  QJsonArray textArray;
  int txt_size = 0;

  for (int i = 0; i < msgList.size(); ++i) {
    //消息内容长度不合规就跳过
    if (msgList[i].content.length() > 1024) {
      continue;
    }

    QString type = msgList[i].msgFlag;
    ChatItemBase *pChatItem = new ChatItemBase(role);
    pChatItem->setUserName(userName);
    pChatItem->setUserIcon(QPixmap(userIcon));
    QWidget *pBubble = nullptr;

    if (type == "text") {
      //生成唯一id
      QUuid uuid = QUuid::createUuid();
      //转为字符串
      QString uuidString = uuid.toString();

      pBubble = new TextBubble(role, msgList[i].content);
      if (txt_size + msgList[i].content.length() > 1024) {
        textObj["fromuid"] = self_info->_uid;
        textObj["touid"] = user_info_->_uid;
        textObj["text_array"] = textArray;
        QJsonDocument doc(textObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
        //发送并清空之前累计的文本列表
        txt_size = 0;
        textArray = QJsonArray();
        textObj = QJsonObject();
        //发送tcp请求给chat server
        emit TcpMgr::getinstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ,
                                                  jsonData);
      }

      // 多条短消息合并，一起发送，TODO，有可能造成消息延迟？

      //将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
      //_bubble_map[uuidString] = pBubble;
      txt_size += msgList[i].content.length();
      QJsonObject obj;
      QByteArray utf8Message = msgList[i].content.toUtf8();
      obj["content"] = QString::fromUtf8(utf8Message);
      obj["msgid"] = uuidString;
      textArray.append(obj);
      auto txt_msg =
          std::make_shared<TextChatData>(uuidString, obj["content"].toString(),
                                         self_info->_uid, user_info_->_uid);
      emit sig_append_send_chat_msg(txt_msg); 
    } else if (type == "image") {
      pBubble = new PictureBubble(QPixmap(msgList[i].content), role);
    } else if (type == "file") {
    }
    //发送消息
    if (pBubble != nullptr) {
      pChatItem->setWidget(pBubble);
      ui->chat_data_list->appendChatItem(pChatItem);
    }
  }

  qDebug() << "textArray is " << textArray;
  //发送给服务器
  textObj["text_array"] = textArray;
  textObj["fromuid"] = self_info->_uid;
  textObj["touid"] = user_info_->_uid;
  QJsonDocument doc(textObj);
  QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
  //发送并清空之前累计的文本列表
  txt_size = 0;
  textArray = QJsonArray();
  textObj = QJsonObject();
  //发送tcp请求给chat server
  emit TcpMgr::getinstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ,
                                            jsonData);
}
