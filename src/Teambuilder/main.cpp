#include "menu.h"
#include <QMainWindow>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("plastique");
    QApplication::setQuitOnLastWindowClosed(false);
    TB_Menu w;

    w.show();
    return a.exec();
}
