#include "mainwindow.h"

#include <QMainWindow>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("plastique");

    MainWindow w;

    w.show();
    return a.exec();
}
