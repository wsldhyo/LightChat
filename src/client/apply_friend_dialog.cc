#include "apply_friend_dialog.hpp"
#include "client_constant.hpp"
#include "tcp_manager.hpp"
#include "ui_applyfrienddlg.h"
#include "usermgr.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QScrollBar>

ApplyFriendDlg::ApplyFriendDlg(QWidget *parent)
    : QDialog(parent), ui(new Ui::ApplyFriendDlg), label_point_(2, 6) {
  ui->setupUi(this);

  // 设置窗口属性：无标题栏、模态对话框
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
  this->setObjectName("ApplyFriendDlg");
  this->setModal(true);

  // 输入框占位符
  ui->name_ed->setPlaceholderText(tr("恋恋风辰")); // 昵称
  ui->back_ed->setPlaceholderText("燃烧的胸毛");   // 备注

  // 标签输入框属性
  ui->lb_ed->setPlaceholderText("搜索、添加标签");
  ui->lb_ed->SetMaxLength(21); // 自定义长度限制
  ui->lb_ed->move(2, 2);
  ui->lb_ed->setFixedHeight(20);
  ui->lb_ed->setMaxLength(10); // Qt 原生长度限制

  // 输入提示框（候选标签列表）
  ui->input_tip_wid->hide();

  // 推荐标签排版初始位置
  tip_cur_point_ = QPoint(5, 5);

  // 推荐标签数据
  tip_data_ = {
      "同学",          "家人",           "菜鸟教程",       "C++ Primer",
      "Rust 程序设计", "父与子学Python", "nodejs开发指南", "go 语言开发指南",
      "游戏伙伴",      "金融投资",       "微信读书",       "拼多多拼友"};

  // “更多”按钮点击 -> 显示全部推荐标签
  connect(ui->more_lb, &ClickedOnceLabel::clicked, this,
          &ApplyFriendDlg::slot_show_more_label);

  // 初始化推荐标签
  init_tip_lbs();

  // 标签输入框事件绑定
  connect(ui->lb_ed, &CustomizeEdit::returnPressed, this,
          &ApplyFriendDlg::slot_label_enter);
  connect(ui->lb_ed, &CustomizeEdit::textChanged, this,
          &ApplyFriendDlg::slot_label_text_change);
  connect(ui->lb_ed, &CustomizeEdit::editingFinished, this,
          &ApplyFriendDlg::slot_label_edit_finished);

  // 输入提示框点击 -> 添加标签
  connect(ui->tip_lb, &ClickedOnceLabel::clicked, this,
          &ApplyFriendDlg::slot_add_firend_label_by_click_tip);

  // 隐藏滚动条，使用事件过滤器动态显示
  ui->scrollArea->horizontalScrollBar()->setHidden(true);
  ui->scrollArea->verticalScrollBar()->setHidden(true);
  ui->scrollArea->installEventFilter(this);

  // 确认/取消按钮状态设置 & 事件绑定
  ui->sure_btn->SetState("normal", "hover", "press");
  ui->cancel_btn->SetState("normal", "hover", "press");
  connect(ui->cancel_btn, &QPushButton::clicked, this,
          &ApplyFriendDlg::slot_apply_cancel);
  connect(ui->sure_btn, &QPushButton::clicked, this,
          &ApplyFriendDlg::slot_apply_sure);
}

ApplyFriendDlg::~ApplyFriendDlg() {
  qDebug() << "ApplyFriendDlg destruct";
  delete ui;
}

void ApplyFriendDlg::init_tip_lbs() {
  int lines = 1;
  for (int i = 0; i < tip_data_.size(); i++) {
    auto *lb = new ClickedLabel(ui->lb_list);
    lb->set_state("normal", "hover", "pressed", "selected_normal",
                  "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(tip_data_[i]);

    // 点击推荐标签 -> 切换好友标签状态
    connect(lb, &ClickedLabel::clicked, this,
            &ApplyFriendDlg::slot_change_friend_label_by_tip);

    // 获取文本宽高，用于排版
    QFontMetrics fontMetrics(lb->font());
    int textWidth = fontMetrics.horizontalAdvance(lb->text());
    int textHeight = fontMetrics.height();
    // 如果一行放不下，就换行
    if (tip_cur_point_.x() + textWidth + g_tip_offset > ui->lb_list->width()) {
      lines++;
      if (lines > 2) { // 限制显示两行
        delete lb;
        return;
      }
      tip_cur_point_.setX(g_tip_offset);
      tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);
    }

    auto next_point = tip_cur_point_;
    add_tip_lbs(lb, tip_cur_point_, next_point, textWidth, textHeight);
    tip_cur_point_ = next_point;
  }
}

void ApplyFriendDlg::add_tip_lbs(ClickedLabel *lb, QPoint cur_point,
                                 QPoint &next_point, int text_width,
                                 int text_height) {
  lb->move(cur_point);
  lb->show();
  add_labels_.insert(lb->text(), lb);
  add_label_keys_.push_back(lb->text());
  // 更新下一个排版起始点
  next_point.setX(lb->pos().x() + text_width + 15);
  next_point.setY(lb->pos().y());
}

bool ApplyFriendDlg::eventFilter(QObject *obj, QEvent *event) {
  if (obj == ui->scrollArea && event->type() == QEvent::Enter) {
    ui->scrollArea->verticalScrollBar()->setHidden(false);
  } else if (obj == ui->scrollArea && event->type() == QEvent::Leave) {
    ui->scrollArea->verticalScrollBar()->setHidden(true);
  }
  return QObject::eventFilter(obj, event);
}

void ApplyFriendDlg::set_search_info(std::shared_ptr<SearchInfo> si) {
  si_ = si;
  auto applyname = UserMgr::get_instance()->get_name();
  auto bakname = si->_name;
  ui->name_ed->setText(applyname);
  ui->back_ed->setText(bakname);
}

void ApplyFriendDlg::slot_show_more_label() {
  qDebug() << "receive more label clicked";
  ui->more_lb_wid->hide();

  // 放宽宽度，重新排版
  ui->lb_list->setFixedWidth(325);
  tip_cur_point_ = QPoint(5, 5);
  auto next_point = tip_cur_point_;
  int textWidth;
  int textHeight;

  // 重排已有推荐标签
  for (auto &added_key : add_label_keys_) {
    auto added_lb = add_labels_[added_key];
    QFontMetrics fontMetrics(added_lb->font());
    textWidth = fontMetrics.horizontalAdvance(added_lb->text());
    textHeight = fontMetrics.height();

    if (tip_cur_point_.x() + textWidth + g_tip_offset > ui->lb_list->width()) {
      tip_cur_point_.setX(g_tip_offset);
      tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);
    }
    added_lb->move(tip_cur_point_);

    next_point.setX(added_lb->pos().x() + textWidth + 15);
    next_point.setY(tip_cur_point_.y());
    tip_cur_point_ = next_point;
  }

  // 添加原来未显示的推荐标签
  for (int i = 0; i < tip_data_.size(); i++) {
    auto iter = add_labels_.find(tip_data_[i]);
    if (iter != add_labels_.end())
      continue;

    auto *lb = new ClickedLabel(ui->lb_list);
    lb->set_state("normal", "hover", "pressed", "selected_normal",
                  "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(tip_data_[i]);
    connect(lb, &ClickedLabel::clicked, this,
            &ApplyFriendDlg::slot_change_friend_label_by_tip);

    QFontMetrics fontMetrics(lb->font());
    int textWidth = fontMetrics.horizontalAdvance(lb->text());
    int textHeight = fontMetrics.height();

    if (tip_cur_point_.x() + textWidth + g_tip_offset > ui->lb_list->width()) {
      tip_cur_point_.setX(g_tip_offset);
      tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);
    }

    next_point = tip_cur_point_;
    add_tip_lbs(lb, tip_cur_point_, next_point, textWidth, textHeight);
    tip_cur_point_ = next_point;
  }

  // 调整容器高度
  int diff_height =
      next_point.y() + textHeight + g_tip_offset - ui->lb_list->height();
  ui->lb_list->setFixedHeight(next_point.y() + textHeight + g_tip_offset);
  ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

void ApplyFriendDlg::reset_labels() {
  auto max_width = ui->gridWidget->width(); // 标签容器宽度
  auto label_height = 0;

  // 遍历现有标签并重新排布
  for (auto iter = friend_labels_.begin(); iter != friend_labels_.end();
       iter++) {
    // 如果超出当前行宽度 -> 换行
    if (label_point_.x() + iter.value()->width() > max_width) {
      label_point_.setY(label_point_.y() + iter.value()->height() + 6);
      label_point_.setX(2);
    }

    // 移动标签到新位置
    iter.value()->move(label_point_);
    iter.value()->show();

    // 更新下一个排布点
    label_point_.setX(label_point_.x() + iter.value()->width() + 2);
    label_point_.setY(label_point_.y());
    label_height = iter.value()->height();
  }

  // 没有标签时 -> 输入框放在起始位置
  if (friend_labels_.isEmpty()) {
    ui->lb_ed->move(label_point_);
    return;
  }

  // 输入框排布逻辑：若剩余宽度不足 -> 换行
  if (label_point_.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()) {
    ui->lb_ed->move(2, label_point_.y() + label_height + 6);
  } else {
    ui->lb_ed->move(label_point_);
  }
}

void ApplyFriendDlg::add_label(QString name) {
  // 避免重复添加
  if (friend_labels_.find(name) != friend_labels_.end()) {
    return;
  }

  // 创建标签控件
  auto tmplabel = new FriendLabel(ui->gridWidget);
  tmplabel->set_text(name);
  tmplabel->setObjectName("FriendLabel");

  auto max_width = ui->gridWidget->width();
  // 如果宽度超出 -> 换行
  if (label_point_.x() + tmplabel->width() > max_width) {
    label_point_.setY(label_point_.y() + tmplabel->height() + 6);
    label_point_.setX(2);
  }

  // 放置标签
  tmplabel->move(label_point_);
  tmplabel->show();

  // 保存到集合
  friend_labels_[tmplabel->text()] = tmplabel;
  friend_label_keys_.push_back(tmplabel->text());

  // 绑定删除信号
  connect(tmplabel, &FriendLabel::sig_close, this,
          &ApplyFriendDlg::slot_remove_friend_label);

  // 更新下一个排布点
  label_point_.setX(label_point_.x() + tmplabel->width() + 2);

  // 调整输入框位置
  if (label_point_.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()) {
    ui->lb_ed->move(2, label_point_.y() + tmplabel->height() + 2);
  } else {
    ui->lb_ed->move(label_point_);
  }

  // 清空输入框
  ui->lb_ed->clear();

  // 动态调整容器高度，保证能容纳新标签
  if (ui->gridWidget->height() < label_point_.y() + tmplabel->height() + 2) {
    ui->gridWidget->setFixedHeight(label_point_.y() + tmplabel->height() * 2 +
                                   2);
  }
}

void ApplyFriendDlg::slot_label_enter() {
  if (ui->lb_ed->text().isEmpty()) {
    return; // 空输入直接忽略
  }

  auto text = ui->lb_ed->text();
  add_label(text); // 添加到好友标签

  ui->input_tip_wid->hide();

  // 推荐标签数据中不存在 -> 将输入内容作为新候选标签
  auto find_it = std::find(tip_data_.begin(), tip_data_.end(), text);
  if (find_it == tip_data_.end()) {
    tip_data_.push_back(text);
  }

  // 推荐标签展示区存在该标签 -> 设置为选中
  auto find_add = add_labels_.find(text);
  if (find_add != add_labels_.end()) {
    find_add.value()->set_cur_state(ClickLbState::Selected);
    return;
  }

  // 推荐标签展示区没有 -> 新建并加入
  auto *lb = new ClickedLabel(ui->lb_list);
  lb->set_state("normal", "hover", "pressed", "selected_normal",
                "selected_hover", "selected_pressed");
  lb->setObjectName("tipslb");
  lb->setText(text);

  connect(lb, &ClickedLabel::clicked, this,
          &ApplyFriendDlg::slot_change_friend_label_by_tip);

  // 计算标签尺寸，用于排版
  QFontMetrics fontMetrics(lb->font());
  int textWidth = fontMetrics.horizontalAdvance(lb->text());
  int textHeight = fontMetrics.height();

  // 如果一行放不下 -> 换行
  if (tip_cur_point_.x() + textWidth + g_tip_offset + 3 >
      ui->lb_list->width()) {
    tip_cur_point_.setX(5);
    tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);
  }

  auto next_point = tip_cur_point_;
  add_tip_lbs(lb, tip_cur_point_, next_point, textWidth, textHeight);
  tip_cur_point_ = next_point;

  // 更新推荐标签区域高度
  int diff_height =
      next_point.y() + textHeight + g_tip_offset - ui->lb_list->height();
  ui->lb_list->setFixedHeight(next_point.y() + textHeight + g_tip_offset);

  lb->set_cur_state(ClickLbState::Selected);
  ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

void ApplyFriendDlg::slot_remove_friend_label(QString name) {
  qDebug() << "receive close signal";

  // 重置起始排版点
  label_point_.setX(2);
  label_point_.setY(6);

  // 查找标签
  auto find_iter = friend_labels_.find(name);
  if (find_iter == friend_labels_.end()) {
    return;
  }

  // 从 key 列表中移除
  auto find_key = friend_label_keys_.end();
  for (auto iter = friend_label_keys_.begin(); iter != friend_label_keys_.end();
       iter++) {
    if (*iter == name) {
      find_key = iter;
      break;
    }
  }
  if (find_key != friend_label_keys_.end()) {
    friend_label_keys_.erase(find_key);
  }

  // 删除标签控件
  delete find_iter.value();
  friend_labels_.erase(find_iter);

  // 重新布局剩余标签
  reset_labels();

  // 推荐标签区存在此标签 -> 恢复为 normal 状态
  auto find_add = add_labels_.find(name);
  if (find_add != add_labels_.end()) {
    find_add.value()->reset_normal_state();
  }
}

void ApplyFriendDlg::slot_change_friend_label_by_tip(QString lbtext,
                                                     ClickLbState state) {
  auto find_iter = add_labels_.find(lbtext);
  if (find_iter == add_labels_.end()) {
    return;
  }
  // 点击候选数据，改变其状态，根据候选数据的状态决定是否将其添加到好友标签中
  if (state == ClickLbState::Selected) {
    //添加到好友标签
    add_label(lbtext);
    return;
  }

  if (state == ClickLbState::Normal) {
    // 从好友标签删除
    slot_remove_friend_label(lbtext);
    return;
  }
}

void ApplyFriendDlg::slot_label_text_change(const QString &text) {
  if (text.isEmpty()) {
    // 输入为空 -> 清空提示，隐藏提示框
    ui->tip_lb->setText("");
    ui->input_tip_wid->hide();
    return;
  }

  // 如果输入的标签不在推荐列表中 -> 显示“添加：xxx”
  auto iter = std::find(tip_data_.begin(), tip_data_.end(), text);
  if (iter == tip_data_.end()) {
    auto new_text = g_add_prefix + text;
    ui->tip_lb->setText(new_text);
    ui->input_tip_wid->show();
    return;
  }

  // 输入内容已存在于推荐标签中 -> 直接显示
  ui->tip_lb->setText(text);
  ui->input_tip_wid->show();
}

void ApplyFriendDlg::slot_label_edit_finished() {
  // 输入框失去焦点时 -> 隐藏提示框
  ui->input_tip_wid->hide();
}

void ApplyFriendDlg::slot_add_firend_label_by_click_tip(QString text) {
  // 如果提示文字包含“添加：”，提取实际标签名
  int index = text.indexOf(g_add_prefix);
  if (index != -1) {
    text = text.mid(index + g_add_prefix.length());
  }

  // 添加到好友标签
  add_label(text);

  // 推荐标签数据中不存在 -> 新增
  auto find_it = std::find(tip_data_.begin(), tip_data_.end(), text);
  if (find_it == tip_data_.end()) {
    tip_data_.push_back(text);
  }

  // 推荐标签展示区存在 -> 设置为选中
  auto find_add = add_labels_.find(text);
  if (find_add != add_labels_.end()) {
    find_add.value()->set_cur_state(ClickLbState::Selected);
    return;
  }

  // 推荐标签展示区没有 -> 新建并设置选中状态
  auto *lb = new ClickedLabel(ui->lb_list);
  lb->set_state("normal", "hover", "pressed", "selected_normal",
                "selected_hover", "selected_pressed");
  lb->setObjectName("tipslb");
  lb->setText(text);

  connect(lb, &ClickedLabel::clicked, this,
          &ApplyFriendDlg::slot_change_friend_label_by_tip);

  // 计算标签尺寸
  QFontMetrics fontMetrics(lb->font());
  int textWidth = fontMetrics.horizontalAdvance(lb->text());
  int textHeight = fontMetrics.height();

  // 放不下则换行
  if (tip_cur_point_.x() + textWidth + g_tip_offset + 3 >
      ui->lb_list->width()) {
    tip_cur_point_.setX(5);
    tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);
  }

  // 添加并更新布局
  auto next_point = tip_cur_point_;
  add_tip_lbs(lb, tip_cur_point_, next_point, textWidth, textHeight);
  tip_cur_point_ = next_point;

  // 更新容器高度
  int diff_height =
      next_point.y() + textHeight + g_tip_offset - ui->lb_list->height();
  ui->lb_list->setFixedHeight(next_point.y() + textHeight + g_tip_offset);

  // 设置为选中状态
  lb->set_cur_state(ClickLbState::Selected);

  ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

void ApplyFriendDlg::slot_apply_cancel() {
  qDebug() << "Slot Apply Cancel";
  // 点击取消 -> 隐藏窗口并销毁
  this->hide();
  deleteLater();
}

void ApplyFriendDlg::slot_apply_sure() {
  qDebug() << "Slot Apply Sure called";
  // 点击确认 -> ，向服务器发起请求
  // 申请人的数据
  QJsonObject jsonObj;
  auto uid = UserMgr::get_instance()->get_uid();
  jsonObj["uid"] = uid;
  auto name = ui->name_ed->text();
  if (name.isEmpty()) {
    // 如果name_ed为空，则使用占位文本作为名称
    name = ui->name_ed->placeholderText();
  }

  jsonObj["applyname"] = name; //名称
  auto bakname = ui->back_ed->text();
  if (bakname.isEmpty()) {
    bakname = ui->back_ed->placeholderText();
  }
  jsonObj["bakname"] = bakname; // 添加成功后，对方的备注名
  jsonObj["touid"] = si_->_uid; // 对方uid

  QJsonDocument doc(jsonObj);
  QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

  //发送tcp请求给chat server
  emit TcpMgr::get_instance()->sig_send_data(ReqId::ID_APPLY_FRIEND_REQ,
                                            jsonData);
  this->hide();
  deleteLater();
}
