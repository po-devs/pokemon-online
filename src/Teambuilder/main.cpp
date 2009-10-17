#include "teambuilder.h"

QString test (const char *entry)
{
    return MoveInfo::Name(MoveInfo::Number(entry));
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TeamBuilder w;

    w.show();
    return a.exec();
}
