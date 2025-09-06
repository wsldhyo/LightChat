#include "apply_friend_dialog.hpp"
#include "ui_applyfrienddlg.h"
#include <QDebug>
#include <QScrollBar>
#include "usermgr.hpp"
#include "client_constant.hpp"

ApplyFriendDlg::ApplyFriendDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ApplyFriendDlg),label_point_(2,6)
{
    ui->setupUi(this);
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("ApplyFriendDlg");
    this->setModal(true);
    ui->name_ed->setPlaceholderText(tr("恋恋风辰"));
    ui->lb_ed->setPlaceholderText("搜索、添加标签");
    ui->back_ed->setPlaceholderText("燃烧的胸毛");

    ui->lb_ed->SetMaxLength(21);
    ui->lb_ed->move(2, 2);
    ui->lb_ed->setFixedHeight(20);
    ui->lb_ed->setMaxLength(10);
    ui->input_tip_wid->hide();

    tip_cur_point_ = QPoint(5, 5);

    tip_data_ = { "同学","家人","菜鸟教程","C++ Primer","Rust 程序设计",
                             "父与子学Python","nodejs开发指南","go 语言开发指南",
                                "游戏伙伴","金融投资","微信读书","拼多多拼友" };

    connect(ui->more_lb, &ClickedOnceLabel::clicked, this, &ApplyFriendDlg::slot_show_more_label);
    init_tip_lbs();
    //链接输入标签回车事件
    connect(ui->lb_ed, &CustomizeEdit::returnPressed, this, &ApplyFriendDlg::slot_label_enter);
    connect(ui->lb_ed, &CustomizeEdit::textChanged, this, &ApplyFriendDlg::slot_label_text_change);
    connect(ui->lb_ed, &CustomizeEdit::editingFinished, this, &ApplyFriendDlg::slot_label_edit_finished);
    connect(ui->tip_lb, &ClickedOnceLabel::clicked, this, &ApplyFriendDlg::slot_add_firend_label_by_click_tip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    ui->sure_btn->SetState("normal","hover","press");
    ui->cancel_btn->SetState("normal","hover","press");
    //连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ApplyFriendDlg::slot_apply_cancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &ApplyFriendDlg::slot_apply_sure);
}

ApplyFriendDlg::~ApplyFriendDlg()
{
    qDebug()<< "ApplyFriendDlg destruct";
    delete ui;
}


void ApplyFriendDlg::init_tip_lbs()
{
    int lines = 1;
    for(int i = 0; i < tip_data_.size(); i++){

        auto* lb = new ClickedLabel(ui->lb_list);
        lb->set_state("normal", "hover", "pressed", "selected_normal",
            "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(tip_data_[i]);
        connect(lb, &ClickedLabel::clicked, this, &ApplyFriendDlg::slot_change_friend_label_by_tip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (tip_cur_point_.x() + textWidth + g_tip_offset > ui->lb_list->width()) {
            lines++;
            if (lines > 2) {
                delete lb;
                return;
            }

            tip_cur_point_.setX(g_tip_offset);
            tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);

        }

       auto next_point = tip_cur_point_;

       add_tip_lbs(lb, tip_cur_point_,next_point, textWidth, textHeight);

       tip_cur_point_ = next_point;
    }

}

void ApplyFriendDlg::add_tip_lbs(ClickedLabel* lb, QPoint cur_point, QPoint& next_point, int text_width, int text_height)
{
    lb->move(cur_point);
    lb->show();
    add_labels_.insert(lb->text(), lb);
    add_label_keys_.push_back(lb->text());
    next_point.setX(lb->pos().x() + text_width + 15);
    next_point.setY(lb->pos().y());
}

bool ApplyFriendDlg::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->scrollArea && event->type() == QEvent::Enter)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}

void ApplyFriendDlg::set_search_info(std::shared_ptr<SearchInfo> si)
{
    si_ = si;
    auto applyname = UserMgr::getinstance()->get_name();
    auto bakname = si->_name;
    ui->name_ed->setText(applyname);
    ui->back_ed->setText(bakname);
}

void ApplyFriendDlg::slot_show_more_label()
{
    qDebug()<< "receive more label clicked";
    ui->more_lb_wid->hide();

    ui->lb_list->setFixedWidth(325);
    tip_cur_point_ = QPoint(5, 5);
    auto next_point = tip_cur_point_;
    int textWidth;
    int textHeight;
    //重拍现有的label
    for(auto & added_key : add_label_keys_){
        auto added_lb = add_labels_[added_key];

        QFontMetrics fontMetrics(added_lb->font()); // 获取QLabel控件的字体信息
        textWidth = fontMetrics.horizontalAdvance(added_lb->text()); // 获取文本的宽度
        textHeight = fontMetrics.height(); // 获取文本的高度

        if(tip_cur_point_.x() +textWidth + g_tip_offset > ui->lb_list->width()){
            tip_cur_point_.setX(g_tip_offset);
            tip_cur_point_.setY(tip_cur_point_.y()+textHeight+15);
        }
        added_lb->move(tip_cur_point_);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(tip_cur_point_.y());

        tip_cur_point_ = next_point;

    }

    //添加未添加的
    for(int i = 0; i < tip_data_.size(); i++){
        auto iter = add_labels_.find(tip_data_[i]);
        if(iter != add_labels_.end()){
            continue;
        }

        auto* lb = new ClickedLabel(ui->lb_list);
        lb->set_state("normal", "hover", "pressed", "selected_normal",
            "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(tip_data_[i]);
        connect(lb, &ClickedLabel::clicked, this, &ApplyFriendDlg::slot_change_friend_label_by_tip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (tip_cur_point_.x() + textWidth + g_tip_offset > ui->lb_list->width()) {

            tip_cur_point_.setX(g_tip_offset);
            tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);

        }

         next_point = tip_cur_point_;

        add_tip_lbs(lb, tip_cur_point_, next_point, textWidth, textHeight);

        tip_cur_point_ = next_point;

    }

   int diff_height = next_point.y() + textHeight + g_tip_offset - ui->lb_list->height();
   ui->lb_list->setFixedHeight(next_point.y() + textHeight + g_tip_offset);

    //qDebug()<<"after resize ui->lb_list size is " <<  ui->lb_list->size();
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+diff_height);
}

void ApplyFriendDlg::reset_labels()
{
    auto max_width = ui->gridWidget->width();
    auto label_height = 0;
    for(auto iter = friend_labels_.begin(); iter != friend_labels_.end(); iter++){
        //todo... 添加宽度统计
        if( label_point_.x() + iter.value()->width() > max_width) {
            label_point_.setY(label_point_.y()+iter.value()->height()+6);
            label_point_.setX(2);
        }

        iter.value()->move(label_point_);
        iter.value()->show();

        label_point_.setX(label_point_.x()+iter.value()->width()+2);
        label_point_.setY(label_point_.y());
        label_height = iter.value()->height();
    }

    if(friend_labels_.isEmpty()){
         ui->lb_ed->move(label_point_);
         return;
    }

    if(label_point_.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()){
        ui->lb_ed->move(2,label_point_.y()+label_height+6);
    }else{
         ui->lb_ed->move(label_point_);
    }
}

void ApplyFriendDlg::add_label(QString name)
{
    if (friend_labels_.find(name) != friend_labels_.end()) {
        return;
    }

    auto tmplabel = new FriendLabel(ui->gridWidget);
    tmplabel->set_text(name);
    tmplabel->setObjectName("FriendLabel");

    auto max_width = ui->gridWidget->width();
    //todo... 添加宽度统计
    if (label_point_.x() + tmplabel->width() > max_width) {
        label_point_.setY(label_point_.y() + tmplabel->height() + 6);
        label_point_.setX(2);
    }
    else {

    }


    tmplabel->move(label_point_);
    tmplabel->show();
    friend_labels_[tmplabel->text()] = tmplabel;
    friend_label_keys_.push_back(tmplabel->text());

    connect(tmplabel, &FriendLabel::sig_close, this, &ApplyFriendDlg::slot_remove_friend_label);

    label_point_.setX(label_point_.x() + tmplabel->width() + 2);

    if (label_point_.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()) {
        ui->lb_ed->move(2, label_point_.y() + tmplabel->height() + 2);
    }
    else {
        ui->lb_ed->move(label_point_);
    }

    ui->lb_ed->clear();

    if (ui->gridWidget->height() < label_point_.y() + tmplabel->height() + 2) {
        ui->gridWidget->setFixedHeight(label_point_.y() + tmplabel->height() * 2 + 2);
    }
}


void ApplyFriendDlg::slot_label_enter()
{
    if(ui->lb_ed->text().isEmpty()){
        return;
    }

    auto text = ui->lb_ed->text();
    add_label(ui->lb_ed->text());

    ui->input_tip_wid->hide();
    auto find_it = std::find(tip_data_.begin(), tip_data_.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == tip_data_.end()) {
        tip_data_.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = add_labels_.find(text);
    if (find_add != add_labels_.end()) {
        find_add.value()->set_cur_state(ClickLbState::Selected);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->lb_list);
    lb->set_state("normal", "hover", "pressed", "selected_normal",
        "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this, &ApplyFriendDlg::slot_change_friend_label_by_tip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "tip_cur_point_.x() is " << tip_cur_point_.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (tip_cur_point_.x() + textWidth + g_tip_offset + 3 > ui->lb_list->width()) {

        tip_cur_point_.setX(5);
        tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);

    }

    auto next_point = tip_cur_point_;

    add_tip_lbs(lb, tip_cur_point_, next_point, textWidth, textHeight);
    tip_cur_point_ = next_point;

    int diff_height = next_point.y() + textHeight + g_tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + g_tip_offset);

    lb->set_cur_state(ClickLbState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

void ApplyFriendDlg::slot_remove_friend_label(QString name)
{
    qDebug() << "receive close signal";

    label_point_.setX(2);
    label_point_.setY(6);

   auto find_iter = friend_labels_.find(name);

   if(find_iter == friend_labels_.end()){
       return;
   }

   auto find_key = friend_label_keys_.end();
   for(auto iter = friend_label_keys_.begin(); iter != friend_label_keys_.end();
       iter++){
       if(*iter == name){
           find_key = iter;
           break;
       }
   }

   if(find_key != friend_label_keys_.end()){
      friend_label_keys_.erase(find_key);
   }


   delete find_iter.value();

   friend_labels_.erase(find_iter);

   reset_labels();

   auto find_add = add_labels_.find(name);
   if(find_add == add_labels_.end()){
        return;
   }

   find_add.value()->reset_normal_state();
}

//点击标已有签添加或删除新联系人的标签
void ApplyFriendDlg::slot_change_friend_label_by_tip(QString lbtext, ClickLbState state)
{
    auto find_iter = add_labels_.find(lbtext);
    if(find_iter == add_labels_.end()){
        return;
    }

    if(state == ClickLbState::Selected){
        //编写添加逻辑
        add_label(lbtext);
        return;
    }

    if(state == ClickLbState::Normal){
        //编写删除逻辑
        slot_remove_friend_label(lbtext);
        return;
    }

}

void ApplyFriendDlg::slot_label_text_change(const QString& text)
{
    if (text.isEmpty()) {
        ui->tip_lb->setText("");
        ui->input_tip_wid->hide();
        return;
    }

    auto iter = std::find(tip_data_.begin(), tip_data_.end(), text);
    if (iter == tip_data_.end()) {
        auto new_text = g_add_prefix + text;
        ui->tip_lb->setText(new_text);
        ui->input_tip_wid->show();
        return;
    }
    ui->tip_lb->setText(text);
    ui->input_tip_wid->show();
}


void ApplyFriendDlg::slot_label_edit_finished()
{
    ui->input_tip_wid->hide();
}

void ApplyFriendDlg::slot_add_firend_label_by_click_tip(QString text)
{
    int index = text.indexOf(g_add_prefix);
    if (index != -1) {
        text = text.mid(index + g_add_prefix.length());
    }
    add_label(text);

    auto find_it = std::find(tip_data_.begin(), tip_data_.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == tip_data_.end()) {
        tip_data_.push_back(text);
    }
   
    //判断标签展示栏是否有该标签
    auto find_add = add_labels_.find(text);
    if (find_add != add_labels_.end()) {
        find_add.value()->set_cur_state(ClickLbState::Selected);
        return;
    }
     
    //标签展示栏也增加一个标签, 并设置绿色选中
	auto* lb = new ClickedLabel(ui->lb_list);
	lb->set_state("normal", "hover", "pressed", "selected_normal",
		"selected_hover", "selected_pressed");
	lb->setObjectName("tipslb");
	lb->setText(text);
	connect(lb, &ClickedLabel::clicked, this, &ApplyFriendDlg::slot_change_friend_label_by_tip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "tip_cur_point_.x() is " << tip_cur_point_.x();
   
	QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
	int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
	int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

	if (tip_cur_point_.x() + textWidth+ g_tip_offset+3 > ui->lb_list->width()) {

		tip_cur_point_.setX(5);
		tip_cur_point_.setY(tip_cur_point_.y() + textHeight + 15);

	}

	auto next_point = tip_cur_point_;

	 add_tip_lbs(lb, tip_cur_point_, next_point, textWidth,textHeight);
	tip_cur_point_ = next_point;

    int diff_height = next_point.y() + textHeight + g_tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + g_tip_offset);

    lb->set_cur_state(ClickLbState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+ diff_height );
}

void ApplyFriendDlg::slot_apply_cancel()
{
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}

void ApplyFriendDlg::slot_apply_sure()
{
    qDebug()<<"Slot Apply Sure called" ;
    this->hide();
    deleteLater();
}