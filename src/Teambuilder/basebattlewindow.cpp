#include "basebattlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "theme.h"
#include "logmanager.h"
#include "../BattleManager/advancedbattledata.h"
#include "../BattleManager/battleclientlog.h"
#include "../BattleManager/battleinput.h"
#include "poketextedit.h"
#include "../Shared/battlecommands.h"
#include "../Utilities/coreclasses.h"
#ifdef QT5
#include <QApplication>
#include <QToolTip>
#endif

using namespace BattleCommands;

BaseBattleInfo::BaseBattleInfo(const PlayerInfo &me, const PlayerInfo &opp, int mode, int myself, int opponent)
    : myself(myself), opponent(opponent)
{
    this->mode =  mode;
    if (mode == ChallengeInfo::Doubles) {
        numberOfSlots = 4;
    } else if (mode == ChallengeInfo::Triples) {
        numberOfSlots = 6;
    } else {
        numberOfSlots = 2;
    }

    pInfo[myself] = me;
    pInfo[opponent] = opp;
}

BaseBattleWindow::BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf,
                                   int _ownid) : ignoreSpecs(NoIgnore)
{
    init(me, opponent, conf, _ownid);
}

void BaseBattleWindow::init(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &_conf,
                            int _ownid)
{
    ownid() = _ownid;
    conf() = _conf;
    conf().receivingMode[0] = this->conf().receivingMode[1] = BattleConfiguration::Spectator;
    conf().avatar[0] = me.avatar;
    conf().avatar[1] = opponent.avatar;
    conf().name[0] = me.name;
    conf().name[1] = opponent.name;

    myInfo = new BaseBattleInfo(me, opponent, conf().mode);
    info().gen = conf().gen;

    init();
    show();
}

BaseBattleWindow::BaseBattleWindow()
{
    ignoreSpecs=NoIgnore;
}

void BaseBattleWindow::delay(qint64 msec)
{
    /* The dynamic cast works if called from BaseBattleWindowIns */
    FlowCommandManager<BattleEnum> *ptr = dynamic_cast<FlowCommandManager<BattleEnum> *>(this);

    if (ptr != NULL) {
        ptr->pause();
    }

    if (msec != 0)
        QTimer::singleShot(msec, this, SLOT(undelay()));
}

void BaseBattleWindow::undelay()
{
    /* The dynamic cast works if called from BaseBattleWindowIns */
    FlowCommandManager<BattleEnum> *ptr = dynamic_cast<FlowCommandManager<BattleEnum> *>(this);

    if (ptr != NULL) {
        ptr->unpause();
    }
}

void BaseBattleWindow::init()
{
    SpectatorWindow::init(conf());

    /* The dynamic cast works if called from BaseBattleWindowIns */
    FlowCommandManager<BattleEnum> *ptr = dynamic_cast<FlowCommandManager<BattleEnum> *>(this);

    if (ptr != NULL) {
        addOutput(ptr);
        ptr->deletable = false;
    }

    QObject *ptr2 = dynamic_cast<QObject*>(getBattle());

    if (ptr2) {
        connect(ptr2, SIGNAL(playCry(int)), SLOT(playCry(int)));
    }

    info().data = &data();

    setAttribute(Qt::WA_DeleteOnClose, true);
    QToolTip::setFont(QFont("Verdana",10));

    blankMessage = false;
    battleEnded = false;
    started() = false;

    QString title = tr("%1 vs %2").arg(data().name(0), data().name(1));
    log = LogManager::obj()->createLog(BattleLog, title);
    log->override = Log::OverrideNo; /* By default, no logging enabled */
    replay = LogManager::obj()->createLog(ReplayLog, title);
    replay->override = Log::OverrideNo;

    replayData.data = "battle_logs_v2\n";
    DataStream stream(&replayData.data, QIODevice::Append);
    stream << conf();
    replayData.t.start();

    setWindowTitle(tr("Battle between %1 and %2").arg(name(0), name(1)));

    QHBoxLayout *columns = new QHBoxLayout(this);
    columns->addLayout(mylayout = new QGridLayout());

    mylayout->addWidget(getSceneWidget(), 0, 0, 1, 3);
    QSettings settings;
    bool saveLog = settings.value("Battle/SaveLogs").toBool();
    mylayout->addWidget(saveLogs = new QCheckBox(tr("Save log")), 1, 0, 1, 2);
    saveLogs->setChecked(saveLog);
    mylayout->addWidget(musicOn = new QCheckBox(tr("Music")), 1, 1, 1, 2);
    mylayout->addWidget(flashWhenMoveDone = new QCheckBox(tr("Flash when a move is done")), 1, 2, 1, 2);

    QSettings s;
    musicOn->setChecked(s.value("BattleAudio/PlayMusic").toBool() || s.value("play_battle_cries").toBool());
    flashWhenMoveDone->setChecked(s.value("Battle/FlashOnMove").toBool());

    QVBoxLayout *chat = new QVBoxLayout();
    columns->addLayout(chat);
    chat->addWidget(mychat = getLogWidget());
    mychat->setAutoClear(false);
    chat->addWidget(myline = new QIRCLineEdit());
    QHBoxLayout * buttons = new QHBoxLayout();
    chat->addLayout(buttons);

    buttons->addWidget(mysend = new QPushButton(tr("C&hat")));
    buttons->addWidget(myclose = new QPushButton(tr("&Close")));
    buttons->addWidget(myignore = new QPushButton(tr("&Ignore spectators")));

    connect(musicOn, SIGNAL(toggled(bool)), SLOT(musicPlayStop()));
    connect(myignore, SIGNAL(clicked()), SLOT(ignoreSpectators()));
    connect(myclose, SIGNAL(clicked()), SLOT(clickClose()));
    connect(myline, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(mysend, SIGNAL(clicked()), SLOT(sendMessage()));

    loadSettings(this);

#ifdef QT5
#else
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    mediaObject = new Phonon::MediaObject(this);

    cryOutput = new Phonon::AudioOutput(Phonon::GameCategory, this);
    cryObject = new Phonon::MediaObject(this);

    /* To link both */
    Phonon::createPath(mediaObject, audioOutput);
    Phonon::createPath(cryObject, cryOutput);

    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(enqueueMusic()));
    connect(cryObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(criesProblem(Phonon::State)));
#endif

    undelayOnSounds = true;

    musicPlayStop();
}

bool BaseBattleWindow::musicPlayed() const
{
    return musicOn->isChecked();
}

bool BaseBattleWindow::flashWhenMoved() const
{
    return flashWhenMoveDone->isChecked();
}

void BaseBattleWindow::changeCryVolume(int v)
{
#ifdef QT5
    Q_UNUSED(v);
#else
    cryOutput->setVolume(float(v)/100);
#endif
}

void BaseBattleWindow::changeMusicVolume(int v)
{
#ifdef QT5
    Q_UNUSED(v);
#else
    audioOutput->setVolume(float(v)/100);
#endif
}

void BaseBattleWindow::musicPlayStop()
{
    if (!musicPlayed()) {
        playBattleCries() = false;
        playBattleMusic() = false;
#ifdef QT5
#else
        mediaObject->pause();
#endif
        return;
    }

    QSettings s;
#ifdef QT5
#else
    audioOutput->setVolume(float(s.value("BattleAudio/MusicVolume").toInt())/100);
    cryOutput->setVolume(float(s.value("BattleAudio/CryVolume").toInt())/100);
#endif

    if (musicPlayed()) {
        playBattleCries() = s.value("BattleAudio/PlaySounds").toBool();
        playBattleMusic() = s.value("BattleAudio/PlayMusic").toBool() || !s.value("play_battle_cries").toBool();
    }

    if (!playBattleMusic()) {
        return;
    }

    /* If more than 5 songs, start with a new music, otherwise carry on where it left. */
    QDir directory = QDir(s.value("BattleAudio/MusicDirectory").toString());
    QStringList files = directory.entryList(QStringList() << "*.mp3" << "*.ogg" << "*.wav" << "*.it" << "*.mid" << "*.m4a" << "*.mp4",
                                            QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);

    QStringList tmpSources;

    foreach(QString file, files) {
        tmpSources.push_back(directory.absoluteFilePath(file));
    }

    /* If it's the same musics as before with only 1 file, we start playing again the paused file (would not be nice to restart from the
        start). Otherwise, a random file will be played from the start */
    if (tmpSources == sources && sources.size() == 1) {
#ifdef QT5
#else
        mediaObject->play();
#endif
        return;
    }

    sources = tmpSources;

    if (sources.size() == 0)
        return;

#ifdef QT5
#else
    mediaObject->setCurrentSource(sources[true_rand()%sources.size()]);
    mediaObject->play();
#endif

}

void BaseBattleWindow::enqueueMusic()
{
    if (sources.size() == 0)
        return;
#ifdef QT5
#else
    mediaObject->enqueue(sources[true_rand()%sources.size()]);
#endif
}

#ifdef QT5
#else
void BaseBattleWindow::criesProblem(Phonon::State newState)
{
    /* Phonon is really unundertandable, we can't use the code commented out */
    //    if ((newState == Phonon::ErrorState || newState == Phonon::StoppedState || newState == Phonon::PausedState) && undelayOnSounds) {
    //        undelay();
    //    }
    if (newState != Phonon::PlayingState && newState != Phonon::LoadingState && undelayOnSounds) {
        undelay();
    }
}
#endif

void BaseBattleWindow::playCry(int pokemon)
{
    if (!playBattleCries())
        return;

    delay();

    pokemon = Pokemon::uniqueId(pokemon).pokenum;

    if (!cries.contains(pokemon)) {
        cries.insert(pokemon, PokemonInfo::Cry(pokemon));
    }

    undelayOnSounds = false;
#ifdef QT5
#else
    cryObject->stop();
#endif
    undelayOnSounds = true;

#ifdef QT5
#else
    cryBuffer.close();
    cryBuffer.setBuffer(&cries[pokemon]);
    cryBuffer.open(QIODevice::ReadOnly);

    cryObject->setCurrentSource(&cryBuffer);
    cryObject->play();
#endif

    /* undelay() will automatically be called when the cryObject stops --
        signal/slot connection in BaseBattleWindow::init() */
}

int BaseBattleWindow::player(int spot) const
{
    return spot % 2;
}

int BaseBattleWindow::opponent(int player) const
{
    return !player;
}

QString BaseBattleWindow::name(int spot) const
{
    return data().name(spot);
}

void BaseBattleWindow::checkAndSaveLog()
{
    if (!log) {
        return;
    }
    log->pushList(getLog()->getLog());
    log->pushHtml("</body>");
    replay->setBinary(replayData.data);
    if (saveLogs->isChecked()) {
        log->override = Log::OverrideYes;
        replay->override = Log::OverrideYes;
    }

    log->close();
    log = NULL;
    replay->close();
    replay = NULL;
}

void BaseBattleWindow::closeEvent(QCloseEvent *)
{
    checkAndSaveLog();
    emit closedBW(battleId());
    close();
}

void BaseBattleWindow::close()
{
    writeSettings(this);
    QWidget::close();
}

void BaseBattleWindow::disable()
{
    mysend->setDisabled(true);
    myline->setDisabled(true); 
    checkAndSaveLog();

    getInput()->entryPoint(BattleEnum::BlankMessage);
    auto mess = std::shared_ptr<QString>(new QString(toBoldColor(tr("The window was disabled due to one of the players closing the battle window."), Qt::blue)));
    getInput()->entryPoint(BattleEnum::PrintHtml, &mess);
}

void BaseBattleWindow::clickClose()
{
    emit closedBW(battleId());
    close();
    return;
}

void BaseBattleWindow::sendMessage()
{
    QString message = myline->text();

    if (message.size() != 0) {
        emit battleMessage(battleId(), message);
        myline->clear();
    }
}

void BaseBattleWindow::receiveData(const QByteArray &inf)
{
    if (inf[0] == char(SpectatorChat) && ignoreSpecs != NoIgnore) {
        return;
    }
    if ( (inf[0] == char(BattleChat) || inf[0] == char(EndMessage)) && ignoreSpecs == char(IgnoreAll)) {
        if (inf[1] == char(info().opponent)) {
            return;
        }
    }

    addReplayData(inf);

    SpectatorWindow::receiveData(inf);
}

void BaseBattleWindow::addReplayData(const QByteArray &inf)
{
    DataStream stream(&replayData.data, QIODevice::Append);
    stream << quint32(replayData.t.elapsed()) << inf;
}

void BaseBattleWindow::ignoreSpectators()
{
    ignoreSpecs = ignoreSpecs +1;
    if (ignoreSpecs > IgnoreAll) {
        ignoreSpecs = 0;
    }

    switch (ignoreSpecs) {
    case NoIgnore:
        myignore->setText(tr("&Ignore spectators")); break;
    case IgnoreSpecs:
        myignore->setText(tr("&Ignore everybody")); break;
    case IgnoreAll:
        myignore->setText(tr("Stop &ignoring")); break;
    }
}

void BaseBattleWindow::onSendOut(int, int, ShallowBattlePoke *, bool)
{
    //Plays the battle cry when a pokemon is switched in
    if(!this->window()->isActiveWindow() && flashWhenMoved()) {
        qobject_cast<QApplication*>(qApp)->alert(this, 0);
    }
}

void BaseBattleWindow::onSendBack(int spot, bool)
{
    if(!this->window()->isActiveWindow() && flashWhenMoved()) {
        qobject_cast<QApplication*>(qApp)->alert(this, 0);
    }

    switchToNaught(spot);
}

void BaseBattleWindow::onUseAttack(int, int, bool)
{
    if(!this->window()->isActiveWindow() && flashWhenMoved()) {
        qobject_cast<QApplication*>(qApp)->alert(this, 0);
    }
}

void BaseBattleWindow::onDisconnection()
{
    mychat->insertHtml("<br><i><b>Disconnected from Server!</b></i>");
}

void BaseBattleWindow::onKo(int spot)
{
    switchToNaught(spot);
}

void BaseBattleWindow::onSpectatorJoin(int id, const QString &n)
{
    addSpectator(true, id, n);
}

void BaseBattleWindow::onSpectatorLeave(int id)
{
    addSpectator(false, id);
}

void BaseBattleWindow::onBattleEnd(int, int)
{
    battleEnded = true;
}

void BaseBattleWindow::addSpectator(bool come, int id, const QString &)
{
    if (come) {
        spectators.insert(id);
    } else {
        spectators.remove(id);
    }
}
