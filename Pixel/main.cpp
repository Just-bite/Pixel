#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Pixel");
    w.setWindowIcon(QIcon(":/application_icon/palette.svg"));
    w.show();
    return a.exec();
}
