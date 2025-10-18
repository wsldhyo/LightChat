#include "new_friend_apply_page.hpp"
#include "authen_friend_dlg.hpp"
#include "new_friend_apply_item.hpp"
#include "new_friend_apply_list.hpp"
#include "tcp_manager.hpp"
#include "ui_newfriendapplypage.h"
#include "user_data.hpp"
#include "usermgr.hpp"
#include <QPainter>
#include <QRandomGenerator>
NewFriendApplyPage::NewFriendApplyPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::NewFriendApplyPage) {
  ui->setupUi(this);

  // 当列表控件发出 sig_show_search 信号时，转发给外部
  connect(ui->apply_friend_list, &NewFriendApplyList::sig_show_search, this,
          &NewFriendApplyPage::sig_show_search);

  // 连接 TCP 模块的好友认证响应信号
  connect(TcpMgr::get_instance().get(), &TcpMgr::sig_friend_apply_rsp, this,
          &NewFriendApplyPage::slot_handle_auth_rsp);
}

NewFriendApplyPage::~NewFriendApplyPage() { delete ui; }

// ------------------ 模拟数据 ------------------
static std::vector<QString> strs = {
    "hello world !", "nice to meet u", "New year,new life",
    "You have to love yourself",
    "My love is written in the wind ever since the whole world is you"};

static std::vector<QString> heads = {":/icons/head_1.jpg", ":/icons/head_2.jpg",
                                     ":/icons/head_3.jpg", ":/icons/head_4.jpg",
                                     ":/icons/head_5.jpg"};

static std::vector<QString> names = {"llfc", "zack",   "golang", "cpp",
                                     "java", "nodejs", "python", "rust"};

void NewFriendApplyPage::add_new_apply(std::shared_ptr<AddFriendApply> apply) {
  // 随机选择头像
  //int randomValue = QRandomGenerator::global()->bounded(100);
  //int head_i = randomValue % heads.size();

  // 创建新列表项
  auto *apply_item = new NewFriendApplyItem();
  auto apply_info =
      std::make_shared<ApplyInfo>(apply->from_uid_, apply->name_, apply->desc_,
                                  apply->icon_, apply->name_, 0, 0);
  apply_item->set_info(apply_info);

  // 创建 QListWidgetItem 并设置自定义 widget
  QListWidgetItem *item = new QListWidgetItem;
  item->setSizeHint(apply_item->sizeHint());
  item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
  ui->apply_friend_list->insertItem(0, item); // 插入顶部
  ui->apply_friend_list->setItemWidget(item, apply_item);

  // 显示“添加好友”按钮
  apply_item->show_add_btn(true);

  // 点击“添加”按钮发出的信号（示例中未实现弹窗）
  unauth_items_[apply_item->get_uid()] = apply_item; // 存储未审核列表
  connect(apply_item, &NewFriendApplyItem::sig_auth_friend, this,
          &NewFriendApplyPage::slot_show_auth_friend_dlg);
}

void NewFriendApplyPage::paintEvent(QPaintEvent *event) {
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void NewFriendApplyPage::load_apply_list() {
  // --- 加载真实好友申请 ---
  auto apply_list = UserMgr::get_instance()->get_apply_list();
  bool has_new_apply{false};
  for (auto &apply : apply_list) {
    auto *apply_item = new NewFriendApplyItem();
    apply_item->set_info(apply);

    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(apply_item->sizeHint());
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
    ui->apply_friend_list->insertItem(0, item);
    ui->apply_friend_list->setItemWidget(item, apply_item);

    if (apply->status_) {
      apply_item->show_add_btn(false); // 已处理过的申请隐藏按钮
    } else {
      has_new_apply = true;
      apply_item->show_add_btn(true); // 未处理的显示按钮
      unauth_items_[apply_item->get_uid()] = apply_item; // 存储未审核列表
      // 连接“添加好友”按钮点击信号
      connect(apply_item, &NewFriendApplyItem::sig_auth_friend, this,
              &NewFriendApplyPage::slot_show_auth_friend_dlg);
    }
  }

  if (has_new_apply) {
    emit sig_recv_new_friend_apply();
  }

  {

    // --- 模拟假数据，用于 UI 测试 --- TOOD, 后期可删除
    for (int i = 0; i < 13; i++) {
      int randomValue = QRandomGenerator::global()->bounded(100);
      int str_i = randomValue % strs.size();
      int head_i = randomValue % heads.size();
      int name_i = randomValue % names.size();

      auto *apply_item = new NewFriendApplyItem();
      auto apply = std::make_shared<ApplyInfo>(
          0, names[name_i], strs[str_i], heads[head_i], names[name_i], 0, 1);
      apply_item->set_info(apply);

      QListWidgetItem *item = new QListWidgetItem;
      item->setSizeHint(apply_item->sizeHint());
      item->setFlags(item->flags() & ~Qt::ItemIsEnabled &
                     ~Qt::ItemIsSelectable);
      ui->apply_friend_list->addItem(item);
      ui->apply_friend_list->setItemWidget(item, apply_item);

      connect(apply_item, &NewFriendApplyItem::sig_auth_friend, this,
              &NewFriendApplyPage::slot_show_auth_friend_dlg

      );
    }
  }
}

void NewFriendApplyPage::slot_handle_auth_rsp(
    std::shared_ptr<AuthRsp> auth_rsp) {
  auto uid = auth_rsp->uid_;
  auto find_iter = unauth_items_.find(uid);
  if (find_iter == unauth_items_.end()) {
    qDebug() << "uid:" << uid << "does not in unauth_items";
    return; // 未找到，不处理
  }

  // 如果对方uid是未认证的陌生人，则将添加按钮隐藏，显示"已添加"标签
  find_iter->second->show_add_btn(false);
}

void NewFriendApplyPage::slot_show_auth_friend_dlg(
    std::shared_ptr<ApplyInfo> apply_info) {

  auto auth_friend_dlg = new AuthenFriendDlg(this);
  auth_friend_dlg->setModal(true);
  auth_friend_dlg->set_apply_info(apply_info);
  auth_friend_dlg->show();

  // AuthfriendDlg的确认和取消按钮里的槽函数调用了deleteLater，后续会回收内存
}