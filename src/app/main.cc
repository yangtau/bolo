#include <QApplication>
#include <iostream>

#include "bolo.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  if (argc < 2) return 0;

  auto res = bolo::Bolo::LoadFromJsonFile(argv[1]);
  if (res) {
    auto mybolo = res.value();
    MainWindow w;
    w.mybolo = &mybolo;
    w.setFixedSize(600, 600);
    w.show();

    return a.exec();
  }

  else {
    std::cerr << res.error();
    return 0;
  }
}
