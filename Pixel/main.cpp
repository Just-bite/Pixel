#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Pixel");
    w.setWindowIcon(QIcon(":/application_icon/main_icon.jpg"));
    w.show();
    return a.exec();
}
