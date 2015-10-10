#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowIcon(QIcon(QPixmap(":/64x64/icons/64x64/costs.png")));
    w.show();

    return a.exec();
}
