#include <iostream>
#include <QApplication>
#include <QPushButton>

int main(int argc, char **argv) {

  std::cout << "hello" << std::endl;
  QApplication app (argc, argv);
  QPushButton button;
  button.setText("My text");
  button.setToolTip("A tooltip");
  button.show();
  return app.exec();
}
