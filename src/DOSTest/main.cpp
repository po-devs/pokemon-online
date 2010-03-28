#include "arg.h"

void IOManager::connectionEstablished()
{
    qDebug() << "Connection established";

    TrainerTeam t;

    QString nick;
    for (int i = 0; i < 10; i++) {
        nick.append('0' + (rand() % 10));
    }

    t.setTrainerNick(nick);

    FullInfo f = {t, true,true,Qt::black};

    a.login(f);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    srand(time(NULL));

    qDebug() << "Random number generator initialized";

    IOManager i;

    return a.exec();
}
