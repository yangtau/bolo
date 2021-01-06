#include "password.h"

#include <QLayout>

PassWord::PassWord(QWidget *parent) : QMessageBox(parent) {
  this->setIcon(QMessageBox::NoIcon);
  this->setWindowTitle("密码");
  this->setStyleSheet("background-color:white");

  password.setParent(this);
  password.setReadOnly(false);
  password.setEchoMode(QLineEdit::Password);

  option_ok = this->addButton("确定", QMessageBox::AcceptRole);
  option_cancel = this->addButton("取消", QMessageBox::AcceptRole);
  plabel = new QLabel("输入密码");

  dynamic_cast<QGridLayout *>(this->layout())->addWidget(plabel, 0, 1, 1, 4);
  dynamic_cast<QGridLayout *>(this->layout())->addWidget(&password, 2, 1, 1, 4);
  dynamic_cast<QGridLayout *>(this->layout())->addWidget(option_ok, 4, 1);
  dynamic_cast<QGridLayout *>(this->layout())->addWidget(option_cancel, 4, 2);

  this->setStyleSheet(
      "QLabel{"
      "min-width: 80px;"
      "min-height: 50px; "
      "}");
};

PassWord::~PassWord(){

};
