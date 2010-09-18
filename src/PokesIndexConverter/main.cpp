#include <QtGui/QApplication>
#include "mainwindow.h"

inline QByteArray getFileContent(const QString &path) {
    QFile f(path);
    f.open(QIODevice::ReadOnly);

    return f.readAll();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
