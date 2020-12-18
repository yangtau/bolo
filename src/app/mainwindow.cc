#include "mainwindow.h"

#include <QCheckBox>
#include <QDebug>
#include <QDragEnterEvent>
#include <QFile>
#include <QLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QTextStream>
#include <QUrl>
#include <iostream>
#include <iterator>
#include <map>

#include "itemdef.h"
#include "itemdelegate.h"
#include "listview.h"
#include "util.h"

MainWindow::MainWindow(std::unique_ptr<bolo::Bolo> &&bolo, QWidget *parent)
    : QMainWindow(parent), mybolo{std::move(bolo)} {
  standard_item_model = new QStandardItemModel(this);
  list_view = new ListView;
  itemdelegate = new ItemDelegate(this);
  sub_layout = new QHBoxLayout;
  main_layout = new QVBoxLayout;

  QWidget *widget = new QWidget();
  this->setCentralWidget(widget);

  //设置列表
  list_view->setItemDelegate(itemdelegate);
  list_view->setSpacing(30);
  list_view->setModel(standard_item_model);
  list_view->setDragEnabled(true);
  list_view->setViewMode(QListView::IconMode);
  list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

  //新建备份文件的按钮
  QPixmap add_image(":/images/Add.jpg");
  new_file.setParent(this);
  new_file.setIcon(add_image);
  new_file.setIconSize(QSize(100, 100));
  new_file.setMinimumSize(100, 100);

  //设置标题
  title.setText("Bolo");
  title.setMinimumSize(200, 100);

  //界面布局
  sub_layout->addWidget(&new_file);
  sub_layout->addStretch();
  sub_layout->addWidget(&title);
  sub_layout->setSizeConstraint(QLayout::SetFixedSize);
  // sub_layout->setContentsMargins(0, 0, 0, 0);

  main_layout->addLayout(sub_layout);
  main_layout->addWidget(list_view);
  main_layout->setStretchFactor(list_view, 1);
  // main_layout->setContentsMargins(0, 0, 0, 0);

  centralWidget()->setLayout(main_layout);

  InitData();

  //开启拖放事件
  this->setAcceptDrops(true);

  connect(&new_file, &QPushButton::released, this, &MainWindow::Show_FileWindow);
  connect(this, &MainWindow::Get_NewFile, this, &MainWindow::Add_NewFile);
  connect(itemdelegate, &ItemDelegate::RequireDetail, this, &MainWindow::Show_FileDetail);
}

MainWindow::~MainWindow() {}

void MainWindow::InitData() {
  for (auto &it : mybolo->backup_files()) {
    ItemData my_itemdata;
    my_itemdata.file_name = QString::fromStdString(it.second.filename);
    my_itemdata.MyBackupFile = it.second;

    QStandardItem *item = new QStandardItem;
    item->setSizeHint(QSize(100, 110));
    item->setData(QVariant::fromValue(my_itemdata), Qt::UserRole);
    standard_item_model->appendRow(item);
  }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls())
    event->acceptProposedAction();
  else
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event) {
  const QMimeData *mime_data = event->mimeData();  // 获取MIME数据
  if (mime_data->hasUrls()) {
    QList<QUrl> url_list = mime_data->urls();  // 获取URL列表

    // 将其中第一个URL表示为本地文件路径
    QString file_name = url_list.at(0).toLocalFile();
    if (!file_name.isEmpty()) {
      emit Get_NewFile(file_name);
    }
  }
}

void MainWindow::Show_FileWindow() {
  file_window.setWindowTitle("本地文件");
  file_window.setAcceptMode(QFileDialog::AcceptOpen);
  file_window.setViewMode(QFileDialog::List);
  file_window.setFileMode(QFileDialog::Directory);

  if (file_window.exec() == QFileDialog::Accepted) {
    QStringList file_names = file_window.selectedFiles();
    emit Get_NewFile(file_names[0]);
  }
}

void MainWindow::Add_NewFile(QString file_path) {
  QMessageBox set_box(QMessageBox::NoIcon, "选项", "", 0, NULL, Qt::Sheet);
  set_box.setStyleSheet("background-color:white");

  QCheckBox option_compress("压缩", &set_box);
  QCheckBox option_encrypt("加密", &set_box);
  QCheckBox option_cloud("云备份", &set_box);

  QPushButton *option_ok = set_box.addButton("确定", QMessageBox::AcceptRole);
  QPushButton *option_cancel = set_box.addButton("取消", QMessageBox::AcceptRole);
  QLabel *pLabel = new QLabel("备份可选项");

  dynamic_cast<QGridLayout *>(set_box.layout())->addWidget(pLabel, 0, 1, 1, 4);
  dynamic_cast<QGridLayout *>(set_box.layout())->addWidget(&option_compress, 2, 1, 1, 4);
  dynamic_cast<QGridLayout *>(set_box.layout())->addWidget(&option_encrypt, 3, 1, 1, 4);
  dynamic_cast<QGridLayout *>(set_box.layout())->addWidget(&option_cloud, 4, 1, 1, 4);
  dynamic_cast<QGridLayout *>(set_box.layout())->addWidget(option_ok, 5, 1);
  dynamic_cast<QGridLayout *>(set_box.layout())->addWidget(option_cancel, 5, 2);

  set_box.exec();

  bool is_compress = option_compress.checkState();
  bool is_encrypt = option_encrypt.checkState();
  // bool is_cloud = option_cloud.checkState();

  if (set_box.clickedButton() == option_ok) {
    auto res = mybolo->Backup(file_path.toStdString(), is_compress, is_encrypt);

    if (!res) {
      std::cerr << res.error();
      return;
    }

    ItemData my_itemdata;
    my_itemdata.file_name = QString::fromStdString(res.value().filename);
    my_itemdata.MyBackupFile = res.value();

    QStandardItem *item = new QStandardItem;
    item->setSizeHint(QSize(100, 110));
    item->setData(QVariant::fromValue(my_itemdata), Qt::UserRole);
    standard_item_model->appendRow(item);
  }

  else
    set_box.close();
}

void MainWindow::Show_FileDetail(const QModelIndex &index) {
  QVariant variant = index.data(Qt::UserRole);
  ItemData data = variant.value<ItemData>();

  QString detail = "";
  detail = detail + "备份文件：" + QString::fromStdString(data.MyBackupFile.filename) + "\n";
  detail = detail + "初始目录：" + QString::fromStdString(data.MyBackupFile.path) + "\n";
  detail = detail + "备份目录：" + QString::fromStdString(data.MyBackupFile.backup_path) + "\n";
  detail = detail + "备份时间：" +
           QString::fromStdString(bolo::TimestampToString(data.MyBackupFile.timestamp)) + "\n";
  detail = detail + "是否压缩：" + (data.MyBackupFile.is_compressed ? "是" : "否") + "\n";
  detail = detail + "是否加密：" + (data.MyBackupFile.is_encrypted ? "是" : "否") + "\n";

  QMessageBox file_detail(
      QMessageBox::NoIcon, "File Detail", detail,
      QMessageBox::Reset | QMessageBox::Save | QMessageBox::Abort | QMessageBox::Close, NULL);
  file_detail.setButtonText(QMessageBox::Reset, QString("恢复备份"));
  file_detail.setButtonText(QMessageBox::Save, QString("更新备份"));
  file_detail.setButtonText(QMessageBox::Abort, QString("删除备份"));
  file_detail.setButtonText(QMessageBox::Close, QString("关闭"));

  file_detail.addButton(QMessageBox::No);
  file_detail.button(QMessageBox::No)->setHidden(true);

  int result = file_detail.exec();
  switch (result) {
    case QMessageBox::Reset: {
      file_window.setWindowTitle("本地文件");
      file_window.setAcceptMode(QFileDialog::AcceptOpen);
      file_window.setViewMode(QFileDialog::List);
      file_window.setFileMode(QFileDialog::Directory);

      if (file_window.exec() == QFileDialog::Accepted) {
        QStringList file_names = file_window.selectedFiles();
        auto res = mybolo->Restore(data.MyBackupFile.id, file_names[0].toStdString());

        if (res) std::cerr << res.error() << std::endl;
      }
      break;
    }
    case QMessageBox::Save: {
      auto res = mybolo->Update(data.MyBackupFile.id);
      if (res) std::cerr << res.error() << std::endl;
      break;
    }
    case QMessageBox::Abort: {
      auto res = mybolo->Remove(data.MyBackupFile.id);
      if (res)
        std::cerr << res.error() << std::endl;
      else
        list_view->model()->removeRow(index.row());
      break;
    }
    case QMessageBox::Close:
      file_detail.close();

    default:
      break;
  }
}
