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
  // set the class member
  standard_item_model = new QStandardItemModel(this);
  list_view = new ListView;
  itemdelegate = new ItemDelegate(this);
  sub_layout = new QHBoxLayout;
  main_layout = new QVBoxLayout;

  // 初始化窗口
  QWidget *widget = new QWidget();
  this->setCentralWidget(widget);

  // 设置列表
  list_view->setItemDelegate(itemdelegate);
  list_view->setSpacing(30);
  list_view->setModel(standard_item_model);
  list_view->setDragEnabled(true);
  list_view->setViewMode(QListView::IconMode);
  list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // 新建备份文件的按钮
  QPixmap add_image(":/images/Add.jpg");
  new_file.setParent(this);
  new_file.setIcon(add_image);
  new_file.setIconSize(QSize(100, 100));
  new_file.setMinimumSize(100, 100);

  // 设置标题
  title.setFont(QFont("Times", 25, QFont::Black));
  title.setText("Bolo");
  title.setMinimumSize(100, 100);

  // 设置进度条
  progressbar.setFixedSize(580, 30);
  progressbar.setRange(0,50000);
  progressbar.setValue(50000);

  // 界面布局
  sub_layout->addWidget(&new_file);
  sub_layout->addStretch();
  sub_layout->addWidget(&title);
  sub_layout->setSizeConstraint(QLayout::SetFixedSize);

  main_layout->addLayout(sub_layout);
  main_layout->addWidget(list_view);
  main_layout->addWidget(&progressbar);

  centralWidget()->setLayout(main_layout);

  // 初始化已存在的备份文件
  InitData();

  // 开启拖放事件
  this->setAcceptDrops(true);

  // 设置信号槽
  connect(&new_file, &QPushButton::released, this,
          &MainWindow::Show_FileWindow);  // 加号显示本地文件列表
  connect(this, &MainWindow::Get_NewFile, this, &MainWindow::Add_NewFile);  // 实施新增备份文件
  connect(itemdelegate, &ItemDelegate::RequireDetail, this,
          &MainWindow::Show_FileDetail);  // 展示备份文件细节，并给出可使用功能
}

MainWindow::~MainWindow() {}

void MainWindow::InitData() {
  for (auto &it : mybolo->backup_files()) {
    // 获取文件属性
    ItemData my_itemdata;
    my_itemdata.file_name = QString::fromStdString(it.second.filename);
    my_itemdata.MyBackupFile = it.second;

    // 添加至管理结构
    QStandardItem *item = new QStandardItem;
    item->setSizeHint(QSize(100, 120));
    item->setData(QVariant::fromValue(my_itemdata), Qt::UserRole);
    item->setToolTip(my_itemdata.file_name);  // 设置鼠标放置显示的文件名
    standard_item_model->appendRow(item);
  }
}

void MainWindow::set_progressbar()
{
  // 控制虚拟进度条读条
  progressbar.setValue(0);
  for(int i = 0; i <= 50000; i++)
    progressbar.setValue(i);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  //获取鼠标所拖动的信息
  if (event->mimeData()->hasUrls())
    event->acceptProposedAction();
  else
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event) {
  // 获取MIME数据
  const QMimeData *mime_data = event->mimeData();

  if (mime_data->hasUrls()) {
    // 获取URL列表
    QList<QUrl> url_list = mime_data->urls();

    // 将其中第一个URL表示为本地文件路径
    QString file_name = url_list.at(0).toLocalFile();
    if (!file_name.isEmpty()) {
      emit Get_NewFile(file_name);
    }
  }
}

void MainWindow::Show_FileWindow() {
  // 设置文件显示窗口属性
  file_window.setWindowTitle("本地文件");
  file_window.setAcceptMode(QFileDialog::AcceptOpen);
  file_window.setViewMode(QFileDialog::List);
  file_window.setFileMode(QFileDialog::Directory);

  // 获取选择文件夹
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

    // 虚拟进度条读条
    set_progressbar();

    auto res = mybolo->Backup(file_path.toStdString(), is_compress, is_encrypt);

    if (!res) {
      std::cerr << res.error();
      return;
    }

    ItemData my_itemdata;
    my_itemdata.file_name = QString::fromStdString(res.value().filename);
    my_itemdata.MyBackupFile = res.value();

    QStandardItem *item = new QStandardItem;
    item->setSizeHint(QSize(100, 120));
    item->setData(QVariant::fromValue(my_itemdata), Qt::UserRole);
    item->setToolTip(my_itemdata.file_name);
    standard_item_model->appendRow(item);
  }

  else
    set_box.close();
}

void MainWindow::Show_FileDetail(const QModelIndex &index) {
  // 获取item存储的数据
  QVariant variant = index.data(Qt::UserRole);
  ItemData data = variant.value<ItemData>();

  // set the detail text
  QString detail = "";
  detail = detail + "备份文件：" + QString::fromStdString(data.MyBackupFile.filename) + "\n";
  detail = detail + "初始目录：" + QString::fromStdString(data.MyBackupFile.path) + "\n";
  detail = detail + "备份目录：" + QString::fromStdString(data.MyBackupFile.backup_path) + "\n";
  detail = detail + "备份时间：" +
           QString::fromStdString(bolo::TimestampToString(data.MyBackupFile.timestamp)) + "\n";
  detail = detail + "是否压缩：" + (data.MyBackupFile.is_compressed ? "是" : "否") + "\n";
  detail = detail + "是否加密：" + (data.MyBackupFile.is_encrypted ? "是" : "否") + "\n";

  // set the detail box
  QMessageBox file_detail(QMessageBox::NoIcon, "File Detail", detail, 0, NULL);

  // set the button
  QPushButton *close_button = file_detail.addButton(tr("关闭"), QMessageBox::AcceptRole);
  QPushButton *delete_button = file_detail.addButton(tr("删除备份"), QMessageBox::AcceptRole);
  QPushButton *update_button = file_detail.addButton(tr("更新备份"), QMessageBox::AcceptRole);
  QPushButton *restore_button = file_detail.addButton(tr("恢复备份"), QMessageBox::AcceptRole);

  // 使右上叉号有效
  file_detail.addButton(QMessageBox::No);
  file_detail.button(QMessageBox::No)->setHidden(true);

  file_detail.exec();
  if (file_detail.clickedButton() == restore_button) {
    // 恢复备份操作
    // 设置并显示文件列表
    file_window.setWindowTitle("本地文件");
    file_window.setAcceptMode(QFileDialog::AcceptOpen);
    file_window.setViewMode(QFileDialog::List);
    file_window.setFileMode(QFileDialog::Directory);

    if (file_window.exec() != QFileDialog::Accepted) 
      file_window.close();
    else
    {
      // 虚拟进度条读条
      set_progressbar();

      QStringList file_names = file_window.selectedFiles();
      auto res = mybolo->Restore(data.MyBackupFile.id, file_names[0].toStdString());

      // 输出错误信息
      if (res) std::cerr << res.error() << std::endl;
    }   
  } else if (file_detail.clickedButton() == update_button) {
    // 更新备份
    // 虚拟进度条读条
    set_progressbar();
    auto res = mybolo->Update(data.MyBackupFile.id);
    // 输出错误信息
    if (res) std::cerr << res.error() << std::endl;
  } else if (file_detail.clickedButton() == delete_button) {
    // 删除备份
    // 进行确认选项，允许用户错误点击
    QMessageBox delete_sure(QMessageBox::Warning, "Warning", "Are you sure to delete this backup?",
                            QMessageBox::Yes | QMessageBox::No, NULL);

    int res = delete_sure.exec();
    if (res == QMessageBox::Yes) {
      // 执行删除操作
      // 虚拟进度条读条
      set_progressbar();
      auto res = mybolo->Remove(data.MyBackupFile.id);
      if (res)
        // 输出错误信息
        std::cerr << res.error() << std::endl;
      else
        // 结构中删除对应数据
        list_view->model()->removeRow(index.row());
    }
  } else if (file_detail.clickedButton() == close_button)
    // 关闭窗口
    file_detail.close();
}
