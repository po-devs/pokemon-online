#include "teambuilder.h"
#include <QMainWindow>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("plastique");
    TeamBuilder w;

    w.show();
    return a.exec();
}
