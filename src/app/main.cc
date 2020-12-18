#include <QApplication>
#include <iostream>
#include <type_traits>

#include "bolo.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <config>" << std::endl;
    return 0;
  }
  auto res = bolo::Bolo::LoadFromJsonFile(argv[1]);
  if (res) {
    MainWindow w(std::move(res.value()));
    w.setFixedSize(600, 600);
    w.show();

    return a.exec();
  } else {
    std::cerr << res.error();
    return 0;
  }
}
