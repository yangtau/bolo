#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "listview.h"
#include "itemdelegate.h"
#include "bolo.h"

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QPushButton>
#include <QDebug>
#include <QLabel>
#include <QListView>
#include <QFileDialog>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bolo::Bolo *mybolo;

private:
    QStandardItemModel *standard_item_model;
    ListView *list_view;
    ItemDelegate *itemdelegate;
    QHBoxLayout *sub_layout;
    QVBoxLayout *main_layout;

    QPushButton new_file;
    QLabel title;
    QFileDialog file_window;

public slots:
    void Show_FileWindow();
    void Add_NewFile(QString file_name);
    void Show_FileDetail(const QModelIndex &index);

public:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void InitData();

signals:
    void Get_NewFile(QString );
};

#endif // MAINWINDOW_H
