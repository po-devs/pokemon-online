#include "arg.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    srand(time(NULL));

    qDebug() << "Random number generator initialized";

    DosManager d;
    (void) d;

    return a.exec();
}
