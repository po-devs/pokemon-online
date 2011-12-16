#include "basebattlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "theme.h"
#include "logmanager.h"
#include "remove_direction_override.h"
#include "spectatorwindow.h"
#include "../BattleManager/battledata.h"
#include "poketextedit.h"

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

    for (int i = 0; i < numberOfSlots; i++) {
        sub.push_back(false);
        pokeAlive.push_back(false);
        specialSprite.push_back(0);
        lastSeenSpecialSprite.push_back(0);
        statChanges.push_back(BattleDynamicInfo());
    }

    pInfo[myself] = me;
    pInfo[opponent] = opp;

    time[myself] = 5*60;
    time[opponent] = 5*60;
    ticking[myself] = false;
    ticking[opponent] = false;
}

BaseBattleWindow::BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf,
                                   int _ownid, Client *client) : delayed(0), ignoreSpecs(NoIgnore), _mclient(client)
{
    ownid() = _ownid;
    this->conf() = conf;
    this->conf().receivingMode[0] = this->conf().receivingMode[1] = BattleConfiguration::Spectator;
    myInfo = new BaseBattleInfo(me, opponent, conf.mode);
    info().gen = conf.gen;

    QSettings s;
    usePokemonNames() = s.value("use_pokemon_names").toBool();

    init();
    show();
}

BaseBattleWindow::BaseBattleWindow()
{
    delayed=0;ignoreSpecs=NoIgnore;
    QSettings s;
    usePokemonNames() = s.value("use_pokemon_names").toBool();
}

void BaseBattleWindow::delay(qint64 msec, bool forceDelay)
{
    delayed += 1;

    if (!forceDelay && delayed > 1)
        delayed = 1;

    if (msec != 0)
        QTimer::singleShot(msec, this, SLOT(undelay()));
}

void BaseBattleWindow::undelay()
{
    if (delayed > 0)
        delayed -= 1;
    else
        return;

    while (delayed == 0 && delayedCommands.size() > 0) {
        QByteArray command = delayedCommands.takeFirst();
        receiveInfo(command);
    }
}

void BaseBattleWindow::init()
{
    test = new SpectatorWindow(conf(), info().name(0), info().name(1));

    setAttribute(Qt::WA_DeleteOnClose, true);
    QToolTip::setFont(QFont("Verdana",10));

    blankMessage = false;
    battleEnded = false;
    started() = false;

    log = LogManager::obj()->createLog(BattleLog, tr("%1 vs %2").arg(info().name(0), info().name(1) + "--"));
    log->override = Log::OverrideNo; /* By default, no logging enabled */

    setWindowTitle(tr("Battle between %1 and %2").arg(name(0), name(1)));

    QHBoxLayout *columns = new QHBoxLayout(this);
    columns->addLayout(mylayout = new QGridLayout());

    mylayout->addWidget(test->getSceneWidget(), 0, 0, 1, 3);
    mylayout->addWidget(saveLogs = new QCheckBox(tr("Save log")), 1, 0, 1, 2);
    mylayout->addWidget(musicOn = new QCheckBox(tr("Music")), 1, 1, 1, 2);
    mylayout->addWidget(flashWhenMoveDone = new QCheckBox(tr("Flash when a move is done")), 1, 2, 1, 2);

    QSettings s;
    musicOn->setChecked(s.value("play_battle_music").toBool());
    flashWhenMoveDone->setChecked(s.value("flash_when_enemy_moves").toBool());

    QVBoxLayout *chat = new QVBoxLayout();
    columns->addLayout(chat);
    chat->addWidget(mychat = test->getLogWidget());
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

    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    mediaObject = new Phonon::MediaObject(this);

    cryOutput = new Phonon::AudioOutput(Phonon::GameCategory, this);
    cryObject = new Phonon::MediaObject(this);

    /* To link both */
    Phonon::createPath(mediaObject, audioOutput);
    Phonon::createPath(cryObject, cryOutput);

    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(enqueueMusic()));
    connect(cryObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(criesProblem(Phonon::State)));

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

void BaseBattleWindow::musicPlayStop()
{
    if (!musicPlayed()) {
        mediaObject->pause();
        return;
    }

    /* If more than 5 songs, start with a new music, otherwise carry on where it left. */
    QSettings s;
    QDir directory = QDir(s.value("battle_music_directory").toString());
    QStringList files = directory.entryList(QStringList() << "*.mp3" << "*.ogg" << "*.wav" << "*.it" << "*.mid",
                                            QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);

    QStringList tmpSources;

    foreach(QString file, files) {
        tmpSources.push_back(directory.absoluteFilePath(file));
    }

    /* If it's the same musics as before with only 1 file, we start playing again the paused file (would not be nice to restart from the
        start). Otherwise, a random file will be played from the start */
    if (tmpSources == sources && sources.size() == 1) {
        mediaObject->play();
        return;
    }

    sources = tmpSources;

    if (sources.size() == 0)
        return;

    mediaObject->setCurrentSource(sources[true_rand()%sources.size()]);

    mediaObject->play();
}

void BaseBattleWindow::enqueueMusic()
{
    if (sources.size() == 0)
        return;
    mediaObject->enqueue(sources[true_rand()%sources.size()]);
}

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

void BaseBattleWindow::playCry(int pokemon)
{
    if (!musicPlayed())
        return;

    delay();

    if (!cries.contains(pokemon)) {
        cries.insert(pokemon, PokemonInfo::Cry(pokemon));
    }

    undelayOnSounds = false;
    cryObject->stop();
    undelayOnSounds = true;

    cryBuffer.close();
    cryBuffer.setBuffer(&cries[pokemon]);
    cryBuffer.open(QIODevice::ReadOnly);

    cryObject->setCurrentSource(&cryBuffer);
    cryObject->play();

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
    return info().name(spot);
}

QString BaseBattleWindow::nick(int player) const
{
    return tr("%1's %2").arg(name(this->player(player)), rnick(player));
}

QString BaseBattleWindow::rnick(int player) const
{
    if (usePokemonNames())
        return PokemonInfo::Name(info().currentShallow(player).num());
    else
        return info().currentShallow(player).nick();
}

void BaseBattleWindow::animateHPBar()
{
    const int spot = animatedHpSpot();
    const int goal = animatedHpGoal();

    QSettings s;
    if (true || !s.value("animate_hp_bar").toBool()) {
        undelay();
        info().currentShallow(spot).lifePercent() = goal;
        return;
    }

    //To stop the commands from being processed
    delay(0, false);

    /* We deal with % hp, 30 msecs per % */
    int life = info().currentShallow(spot).lifePercent();

    if (goal == life) {
        delay(120, false);
        return;
    }

    info().currentShallow(spot).lifePercent() = life < goal ? life+1 : life-1;
    //Recursive call to update the hp bar 30msecs later
    QTimer::singleShot(30, this, SLOT(animateHPBar()));
}

void BaseBattleWindow::checkAndSaveLog()
{
    log->pushHtml("</body>");
    if (saveLogs->isChecked()) {
        log->override = Log::OverrideYes;
    }

    log->close();
    log = NULL;
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

void BaseBattleWindow::clickClose()
{
    emit closedBW(battleId());
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

void BaseBattleWindow::receiveInfo(QByteArray inf)
{
    if (delayed && inf[0] != char(BattleChat) && inf[0] != char(SpectatorChat) && inf[0] != char(ClockStart) && inf[0] != char(ClockStop)
            && inf[0] != char(Spectating)) {
        delayedCommands.push_back(inf);
        return;
    }
    test->receiveData(inf);

    /* At the start of the battle 700 ms are waited, to prevent misclicks
       when wanting to do something else */
    if (!started() && inf[0] == char(OfferChoice)) {
        started() = true;
        delay(700);
    }

    QDataStream in (&inf, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_5);

    uchar command;
    qint8 player;

    in >> command >> player;

    dealWithCommandInfo(in, command, player, player);
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

void BaseBattleWindow::dealWithCommandInfo(QDataStream &in, int command, int spot, int truespot)
{
    switch (command)
    {
    case SendOut:
    {
        bool silent;
        quint8 prevIndex;
        in >> silent;
        in >> prevIndex;

        info().sub[spot] = false;
        info().specialSprite[spot] = Pokemon::NoPoke;

        info().switchPoke(spot, prevIndex);
        in >> info().currentShallow(spot);
        info().pokeAlive[spot] = true;

        //Plays the battle cry when a pokemon is switched in
        if (musicPlayed())
        {
            playCry(info().currentShallow(spot).num().pokenum);
        }
        if(!this->window()->isActiveWindow() && flashWhenMoved()) {
            qApp->alert(this, 0);
        }

        break;
    }
    case SendBack:
        bool silent;
        in >> silent;

        if (silent) {
            break;
        }
        if(!this->window()->isActiveWindow() && flashWhenMoved()) {
            qApp->alert(this, 0);
        }

        switchToNaught(spot);
        break;
    case UseAttack:
    {
        qint16 attack;
        in >> attack;
        if(!this->window()->isActiveWindow() && flashWhenMoved()) {
            qApp->alert(this, 0);
        }

        break;
    }
    case BeginTurn:
    {
        break;
    }
    case ChangeHp:
    {
        quint16 newHp;
        in >> newHp;

        animatedHpSpot() = spot;
        animatedHpGoal() = newHp;
        animateHPBar();
        break;
    }
    case Ko:
        //Plays the battle cry when a pokemon faints
        if (musicPlayed())
        {
            playCry(info().currentShallow(spot).num().pokenum);
        }
        switchToNaught(spot);
        break;
    case Hit:
    {
        break;
    }
    case Effective:
    {
        break;
    }
    case CriticalHit:
        break;
    case Miss:
        break;
    case Avoid:
        break;
    case StatChange:
        break;
    case StatusChange:
    {
        break;
    }
    case AbsStatusChange:
    {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        if (status != Pokemon::Confused) {
            info().pokemons[spot][poke].changeStatus(status);
        }
        break;
    }
    case AlreadyStatusMessage:
    {
        break;
    }
    case StatusMessage:
    {
        break;
    }
    case Failed:
        break;
    case BattleChat:
    case EndMessage:
    {
        break;
    }
    case Spectating:
    {
        bool come;
        qint32 id;
        in >> come >> id;
        addSpectator(come, id);
        break;
    }
    case SpectatorChat:
    {
        break;
    }
    case MoveMessage:
    {
        break;
    }
    case NoOpponent:
        break;
    case ItemMessage:
    {
        break;
    }
    case Flinch:
        break;
    case Recoil:
    {
        break;
    }
    case WeatherMessage:
        break;
    case StraightDamage :
    {
        break;
    }
    case AbilityMessage:
    {
        break;
    }
    case Substitute:
        in >> info().sub[spot];
        break;
    case BattleEnd:
    {
        qint8 res;
        in >> res;
        battleEnded = true;
        break;
    }
    case BlankMessage:
        break;
    case Clause:
    {
        break;
    }
    case Rated:
    {
        break;
    }
    case TierSection:
    {
        break;
    }
    case DynamicInfo:
    {
        in >> info().statChanges[spot];
        break;
    }
    case TempPokeChange:
    {
        quint8 type;
        in >> type;
        if (type == TempSprite) {
            Pokemon::uniqueId old = info().specialSprite[spot];
            in >> info().specialSprite[spot];
            if (info().specialSprite[spot] == -1) {
                info().lastSeenSpecialSprite[spot] = old;
            } else if (info().specialSprite[spot] == Pokemon::NoPoke) {
                info().specialSprite[spot] = info().lastSeenSpecialSprite[spot];
            }
        } else if (type == DefiniteForme)
        {
            quint8 poke;
            quint16 newform;
            in >> poke >> newform;
            info().pokemons[spot][poke].num() = newform;
            if (info().isOut(spot, poke)) {
                info().currentShallow(info().slot(spot, poke)).num() = newform;
            }
        } else if (type == AestheticForme)
        {
            quint16 newforme;
            in >> newforme;
            info().currentShallow(spot).num().subnum = newforme;
        }
        break;
    }
    case ClockStart:
    {
        in >> info().time[spot];
        info().startingTime[spot] = time(NULL);
        info().ticking[spot] = true;
        break;
    }
    case ClockStop:
    {
        in >> info().time[spot];
        info().ticking[spot] = false;
        break;
    }
    case SpotShifts:
    {
        qint8 s1, s2;
        bool silent;

        in >> s1 >> s2 >> silent;

        info().switchOnSide(spot, s1, s2);

        delay(500);
        break;
    }
    default:
        printLine("<i>" + tr("Unknown command received, are you up to date?") + "</i>");
        break;
    }
}

void BaseBattleWindow::switchToNaught(int spot)
{
    info().pokeAlive[spot] = false;
}

void BaseBattleWindow::addSpectator(bool come, int id)
{
    if (come) {
        spectators.insert(id);
    } else {
        spectators.remove(id);
    }
}

void BaseBattleWindow::printLine(const QString &str, bool silent)
{
    if (str == "" && blankMessage) {
        return;
    }

    if (str == "") {
        blankMessage = true;
        mychat->insertHtml("");
    } else if (!silent) {
        blankMessage = false;
    }

    QString html = str + "<br />";
    if (!silent) {
        mychat->insertHtml(removeDirectionOverride(html));
        log->pushHtml(html);
    } else {
        log->pushHtml("<!--"+html+"-->");
    }
}

void BaseBattleWindow::printHtml(const QString &str, bool silent, bool newline)
{
    if (!silent) {
        blankMessage = false;
    }

    QString html = str + (newline?"<br />":"");
    if (!silent) {
        mychat->insertHtml(removeDirectionOverride(html));
        log->pushHtml(html);
    } else {
        log->pushHtml("<!--"+html+"-->");
    }
}
