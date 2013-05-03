#include <QFile>
#include <QGridLayout>
#include <QStyle>
#include <QPushButton>
#include <QApplication>
#include <QMessageBox>
#include "replayviewer.h"
#include "spectatorwindow.h"
#include "poketextedit.h"
#include "../Utilities/coreclasses.h"
#include "../BattleManager/battleinput.h"

/* Format for raw:

 battle_logs_v2<line break>
 <configuration (FullBattleConfiguration)>
 <timestamp (32 bits)><command (byteArray)> * infinite
*/

ReplayViewer::ReplayViewer(const QString &file) : finished(false), paused(false), speeding(false), timerDiff(0), nextRead(quint32(-1))
{
    in = new QFile(file);

    if (!in->open(QFile::ReadOnly)) {
        QMessageBox::critical(NULL, tr("Error when opening replay file"), tr("The replay file couldn't be opened: %1").arg(file));
        deleteLater();
        return;
    }

    QByteArray version = in->readLine();

    if (version != "battle_logs_v2\n") {
        QMessageBox::critical(NULL, tr("Log format not supported"), tr("The replay version of the file isn't supported by this client."));
        deleteLater();
        return;
    }

    DataStream stream(in);

    stream >> conf;

    window = new SpectatorWindow(conf);

    QWidget *widget = new QWidget();
    QGridLayout *gl = new QGridLayout(widget);
    gl->addWidget(window->getSceneWidget(), 0, 0);
    gl->addWidget(window->getLogWidget(), 0, 1);

    bar.init();
    gl->addLayout(bar.getLayout(), 1, 0, 1, 2);
    connect(&bar, SIGNAL(paused()), SLOT(pause()));
    connect(&bar, SIGNAL(speedChange(bool)), SLOT(changeSpeed(bool)));
    connect(&bar, SIGNAL(play()), SLOT(play()));
    connect(&bar, SIGNAL(seekNext()), SLOT(seekNext()));

    widget->setWindowTitle(tr("Pok\303\251mon Online Replay"));

    widget->setObjectName("ReplayViewer");
    widget->show();
    //widget->setWindowFlags(Qt::Window);
    widget->setAttribute(Qt::WA_DeleteOnClose);

    connect(widget, SIGNAL(destroyed()), SLOT(deleteLater()));

    t.start();
    read();
}

void ReplayViewer::read(bool forced)
{
    if (finished) {
        return;
    }
    if (paused && !forced) {
        return;
    }

    /* If the battle animation is animating, we're not going to pile up
      data & data to read. Because we need to retain fine control on
      when to pause/play, and when we pile up data we can't take it back */
    if (window->getInput()->paused()) {
        QTimer::singleShot(20, this, SLOT(read()));
        return;
    }

    DataStream stream(in);

    if (nextRead == quint32(-1)) {
        stream >> nextRead;
        stream >> lastData;
    }

    while(nextRead <= t.elapsed() + timerDiff) {
        if (lastData.size() == 0) {
            //finished
            finished = true;
            window->getInput()->entryPoint(BattleEnum::BlankMessage);
            auto mess = std::shared_ptr<QString>(new QString(toBoldColor(tr("This is the end of the replay"), Qt::blue)));
            window->getInput()->entryPoint(BattleEnum::PrintHtml, &mess);
            return;
        }

        window->receiveData(lastData);

        stream >> nextRead;
        stream >> lastData;
    }

    int delay = nextRead-(t.elapsed()+timerDiff+1);
    if (speeding && delay > 100) {
        QTimer::singleShot(100, this, SLOT(seekNext()));
    } else {
        QTimer::singleShot(delay, this, SLOT(read()));
    }
}

ReplayViewer::~ReplayViewer()
{
    delete in;
    delete window;
}

void ReplayViewer::pause()
{
    paused = true;
    timerDiff += t.elapsed();
}

void ReplayViewer::play()
{
    paused = false;
    t.restart();
    read();
}

void ReplayViewer::changeSpeed(bool fast)
{
    speeding = fast;

    if (fast) {
        QTimer::singleShot(100, this, SLOT(seekNext()));
    }
}

void ReplayViewer::seekNext()
{
    timerDiff = nextRead +1;
    t.restart();
    read(true);
}

void ReplayBar::init()
{
    pause = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaPause),"");
    fastforward = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward), "");
    next = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward), "");

    fastforward->setCheckable(true);

    pause->setProperty("paused", false);

    connect(pause, SIGNAL(clicked()), SLOT(changePause()));
    connect(next, SIGNAL(clicked()), SIGNAL(seekNext()));
    connect(fastforward, SIGNAL(clicked(bool)), SIGNAL(speedChange(bool)));
}

void ReplayBar::changePause()
{
    if (pause->property("paused").toBool()) {
        pause->setProperty("paused", false);
        pause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPause));

        emit play();
    } else {
        pause->setProperty("paused", true);
        pause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));

        emit paused();
    }
}

QHBoxLayout *ReplayBar::getLayout()
{
    QHBoxLayout *l = new QHBoxLayout();
    l->addWidget(pause);
    l->addWidget(next);
    l->addWidget(fastforward);

    return l;
}
