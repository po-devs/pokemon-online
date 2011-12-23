#include <QFile>
#include <QMessageBox>
#include "replayviewer.h"
#include "spectatorwindow.h"
#include "poketextedit.h"
#include "../BattleManager/battleinput.h"

/* Format for raw:

 battle_logs_v1<line break>
 <configuration (FullBattleConfiguration)>
 <timestamp (32 bits)><command (byteArray)> * infinite
*/

ReplayViewer::ReplayViewer(const QString &file) : nextRead(quint32(-1))
{
    in = new QFile(file);

    if (!in->open(QFile::ReadOnly)) {
        QMessageBox::critical(NULL, tr("Error when opening replay file"), tr("The replay file couldn't be opened: %1").arg(file));
        deleteLater();
        return;
    }

    QByteArray version = in->readLine();

    if (version != "battle_logs_v1\n") {
        QMessageBox::critical(NULL, tr("Log format not supported"), tr("The replay version of the file isn't supported by this client."));
        deleteLater();
        return;
    }

    QDataStream stream(in);
    stream.setVersion(QDataStream::Qt_4_7);

    stream >> conf;

    window = new SpectatorWindow(conf);
    window->setParent(this);

    QWidget *widget = window->getSampleWidget();
    widget->setWindowTitle(tr("Pok\303\251mon Online Replay"));

    widget->show();
    //widget->setWindowFlags(Qt::Window);
    widget->setAttribute(Qt::WA_DeleteOnClose);

    connect(widget, SIGNAL(destroyed()), SLOT(deleteLater()));

    t.start();
    read();
}

void ReplayViewer::read()
{
    QDataStream stream(in);
    stream.setVersion(QDataStream::Qt_4_7);

    if (nextRead == quint32(-1)) {
        stream >> nextRead;
        stream >> lastData;
    }

    while(nextRead <= t.elapsed()) {
        if (lastData.size() == 0) {
            //finished
            window->getInput()->entryPoint(BattleEnum::BlankMessage);
            auto mess = std::shared_ptr<QString>(new QString(toBoldColor(tr("This is the end of the replay"), Qt::blue)));
            window->getInput()->entryPoint(BattleEnum::PrintHtml, &mess);
            return;
        }

        window->receiveData(lastData);

        stream >> nextRead;
        stream >> lastData;
    }

    QTimer::singleShot(nextRead-t.elapsed()+1, this, SLOT(read()));
}

ReplayViewer::~ReplayViewer()
{
    delete in;
}
