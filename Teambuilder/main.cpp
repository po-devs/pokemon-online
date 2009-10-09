#include "teambuilder.h"

int main(int argc, char *argv[])
{
    (void) PkInfo;

    QApplication a(argc, argv);
    TeamBuilder w;
    w.show();
    return a.exec();
}
