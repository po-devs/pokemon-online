#include "basebattlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "theme.h"
#include "logmanager.h"
#include "remove_direction_override.h"

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
    myInfo = new BaseBattleInfo(me, opponent, conf.mode);
    info().gen = conf.gen;
    mydisplay = new BaseBattleDisplay(info());
    QSettings s;
    usePokemonNames() = s.value("use_pokemon_names").toBool();

    init();
    show();

    log->pushHtml("<!DOCTYPE html>");
    log->pushHtml("<!-- Pokemon Online battle spectator log (version 1.0) -->");
    if (client) {
        log->pushHtml(QString("<!-- Log belonging to %1-->").arg(client->name(ownid())));
    }
    log->pushHtml(QString("<head>\n\t<title>%1 vs %2</title>\n</head>").arg(info().name(0), info().name(1)));
    log->pushHtml("<body>");
    printHtml(toBoldColor(tr("Battle between %1 and %2 is underway!"), Qt::blue).arg(name(true), name(false)));
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

    mylayout->addWidget(mydisplay, 0, 0, 1, 3);
    mylayout->addWidget(saveLogs = new QCheckBox(tr("Save log")), 1, 0, 1, 2);
    mylayout->addWidget(musicOn = new QCheckBox(tr("Music")), 1, 1, 1, 2);
    mylayout->addWidget(myclose = new QPushButton(tr("&Close")),1,2);

    QSettings s;
    musicOn->setChecked(s.value("play_battle_music").toBool());

    QVBoxLayout *chat = new QVBoxLayout();
    columns->addLayout(chat);
    chat->addWidget(mychat = new QScrollDownTextBrowser());
    mychat->setAutoClear(false);
    chat->addWidget(myline = new QIRCLineEdit());
    QHBoxLayout * buttons = new QHBoxLayout();
    chat->addLayout(buttons);

    buttons->addWidget(mysend = new QPushButton(tr("C&hat")));
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
    if (!s.value("animate_hp_bar").toBool()) {
        undelay();
        info().currentShallow(spot).lifePercent() = goal;
        mydisplay->updatePoke(spot);
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

    mydisplay->updatePoke(spot);
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

bool BaseBattleWindow::hasKnowledgeOf(int player) const
{
    /* Ironically, the battlers id aren't even stored here, so we have to use another way */
    return spectators.contains(player) || client()->name(player) == name(0) || client()->name(player) == name(1);
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
        mydisplay->updatePoke(spot);

        mydisplay->updatePoke(info().player(spot), info().slotNum(spot));
        mydisplay->updatePoke(info().player(spot), prevIndex);

        //Plays the battle cry when a pokemon is switched in
        if (musicPlayed())
        {
            playCry(info().currentShallow(spot).num().pokenum);
        }


        QString pokename = PokemonInfo::Name(info().currentShallow(spot).num());
        if (pokename != rnick(spot))
            printLine(tr("%1 sent out %2! (%3)").arg(name(player(spot)), rnick(spot), pokename), silent);
        else
            printLine(tr("%1 sent out %2!").arg(name(player(spot)), rnick(spot)), silent);

        printLine(tr("%1's previous position in the team: %2.").arg(nick(spot)).arg(prevIndex), true);
        printLine(tr("%1's new place on the field: %2.").arg(nick(spot)).arg(info().slotNum(spot)), true);
        printLine(tr("%1's life: %2%.").arg(nick(spot)).arg(info().currentShallow(spot).lifePercent()), true);
        printLine(tr("%1's status: %2.").arg(nick(spot), StatInfo::Status(info().currentShallow(spot).status())), true);
        printLine(tr("%1's level: %2.").arg(nick(spot)).arg(info().currentShallow(spot).level()), true);
        printLine(tr("%1's shininess: %2.").arg(nick(spot)).arg(info().currentShallow(spot).shiny()), true);
        printLine(tr("%1's gender: %2.").arg(nick(spot)).arg(GenderInfo::Name(info().currentShallow(spot).gender())), true);

        break;
    }
    case SendBack:
        printLine(tr("%1 called %2 back!").arg(name(player(spot)), rnick(spot)));
        switchToNaught(spot);
        break;
    case UseAttack:
    {
        qint16 attack;
        in >> attack;

        printHtml(tr("%1 used %2!").arg(escapeHtml(tu(nick(spot))), toBoldColor(MoveInfo::Name(attack), Theme::TypeColor(MoveInfo::Type(attack, gen())))));
        break;
    }
    case BeginTurn:
    {
        int turn;
        in >> turn;
        printLine("");
        printHtml(toBoldColor(tr("Start of turn %1").arg(turn), Qt::blue));
        break;
    }
    case ChangeHp:
    {
        quint16 newHp;
        in >> newHp;

        printLine(tr("%1's new HP is %2%.").arg(nick(spot)).arg(newHp), true);

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

        printHtml("<b>" + escapeHtml(tu(tr("%1 fainted!").arg(nick(spot)))) + "</b>");
        switchToNaught(spot);
        break;
    case Hit:
    {
        quint8 number;
        in >> number;
        printLine(tr("Hit %1 times!").arg(int(number)));
        break;
    }
    case Effective:
    {
        quint8 eff;
        in >> eff;
        switch (eff) {
        case 0:
            printLine(tr("It had no effect!"));
            break;
        case 1:
        case 2:
            printHtml(toColor(tr("It's not very effective..."), Qt::gray));
            break;
        case 8:
        case 16:
            printHtml(toColor(tr("It's super effective!"), Qt::blue));
        default:
            break;
        }
        break;
    }
    case CriticalHit:
        printHtml(toColor(tr("A critical hit!"), "#6b0000"));
        break;
    case Miss:
        printLine(tr("The attack of %1 missed!").arg(nick(spot)));
        break;
    case Avoid:
        printLine(tr("%1 avoided the attack!").arg(tu(nick(spot))));
        break;
    case StatChange:
        qint8 stat, boost;
        in >> stat >> boost;

        printLine(tu(tr("%1's %2 %3%4!").arg(nick(spot), StatInfo::Stat(stat), abs(boost) > 1 ? tr("sharply ") : "", boost > 0 ? tr("rose") : tr("fell"))));
        break;
    case StatusChange:
    {
        static const QString statusChangeMessages[6] = {
            tr("%1 is paralyzed! It may be unable to move!"),
            tr("%1 fell asleep!"),
            tr("%1 was frozen solid!"),
            tr("%1 was burned!"),
            tr("%1 was poisoned!"),
            tr("%1 was badly poisoned!")
        };

        qint8 status;
        in >> status;
        bool multipleTurns;
        in >> multipleTurns;
        if (status > Pokemon::Fine && status <= Pokemon::Poisoned) {
            printHtml(toColor(tu(statusChangeMessages[status-1 + (status == Pokemon::Poisoned && multipleTurns)].arg(nick(spot))), Theme::StatusColor(status)));
        } else if (status == Pokemon::Confused) {
            printHtml(toColor(escapeHtml(tu(tr("%1 became confused!").arg(nick(spot)))), Theme::TypeColor(Type::Ghost).name()));
        }
        printLine(tr("%1 had its status changed to: %2.").arg(nick(spot), StatInfo::Status(status)), true);

        break;
    }
    case AbsStatusChange:
    {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        printLine(tr("Pokemon number %1 of %2 had its status changed to: %3.").arg(poke).arg(name(spot), StatInfo::Status(status)), true);

        if (status != Pokemon::Confused) {
            info().pokemons[spot][poke].changeStatus(status);
            if (info().isOut(spot, poke))
                mydisplay->updatePoke(info().slot(spot, poke));
        }
        mydisplay->changeStatus(spot,poke,status);
        break;
    }
    case AlreadyStatusMessage:
    {
        quint8 status;
        in >> status;
        printHtml(toColor(tr("%1 is already %2.").arg(tu(nick(spot)), StatInfo::Status(status)),
                          Theme::StatusColor(status)));
        break;
    }
    case StatusMessage:
    {
        qint8 status;
        in >> status;
        switch(status)
        {
        case FeelConfusion:
            printHtml(toColor(escapeHtml(tu(tr("%1 is confused!").arg(nick(spot)))), Theme::TypeColor(Type::Ghost).name()));
            break;
        case HurtConfusion:
            printHtml(toColor(escapeHtml(tu(tr("It hurt itself in its confusion!"))), Theme::TypeColor(Type::Ghost).name()));
            break;
        case FreeConfusion:
            printHtml(toColor(escapeHtml(tu(tr("%1 snapped out its confusion!").arg(nick(spot)))), Theme::TypeColor(Type::Dark).name()));
            break;
        case PrevParalysed:
            printHtml(toColor(escapeHtml(tu(tr("%1 is paralyzed! It can't move!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Paralysed)));
            break;
        case FeelAsleep:
            printHtml(toColor(escapeHtml(tu(tr("%1 is fast asleep!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Asleep)));
            break;
        case FreeAsleep:
            printHtml(toColor(escapeHtml(tu(tr("%1 woke up!").arg(nick(spot)))), Theme::TypeColor(Type::Dark).name()));
            break;
        case HurtBurn:
            printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by its burn!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Burnt)));
            break;
        case HurtPoison:
            printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by poison!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Poisoned)));
            break;
        case PrevFrozen:
            printHtml(toColor(escapeHtml(tu(tr("%1 is frozen solid!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Frozen)));
            break;
        case FreeFrozen:
            printHtml(toColor(escapeHtml(tu(tr("%1 thawed out!").arg(nick(spot)))), Theme::TypeColor(Type::Dark).name()));
            break;
        }
    }
    break;
    case Failed:
        printLine(tr("But it failed!"));
        break;
    case BattleChat:
    case EndMessage:
    {
        if (ignoreSpecs == IgnoreAll && name(spot) != client()->name(ownid()))
            return;
        QString message;
        in >> message;
        if (message=="")
            return;
        printHtml(QString("<span style='color:") + (spot?"#5811b1":"green") + "'><b>" + escapeHtml(name(spot)) + ": </b></span>" + escapeHtml(message));
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
        qint32 id;
        QString message;
        in >> id >> message;
        if (id != ownid() && (ignoreSpecs != NoIgnore))
            return;
        printHtml(toColor(client()->name(id), Qt::blue) + ": " + escapeHtml(message));
        break;
    }
    case MoveMessage:
    {
        quint16 move=0;
        uchar part=0;
        qint8 type(0), foe(0);
        qint16 other(0);
        QString q;
        in >> move >> part >> type >> foe >> other >> q;
        QString mess = MoveInfo::MoveMessage(move,part);
        mess.replace("%s", nick(spot));
        mess.replace("%ts", name(player(spot)));
        mess.replace("%tf", name(opponent(player(spot))));
        mess.replace("%t", TypeInfo::Name(type));
        mess.replace("%f", nick(foe));
        mess.replace("%m", MoveInfo::Name(other));
        mess.replace("%d", QString::number(other));
        mess.replace("%q", q);
        mess.replace("%i", ItemInfo::Name(other));
        mess.replace("%a", AbilityInfo::Name(other));
        mess.replace("%p", PokemonInfo::Name(other));
        printHtml(toColor(escapeHtml(tu(mess)), Theme::TypeColor(type)));
        break;
    }
    case NoOpponent:
        printLine(tr("But there was no target..."));
        break;
    case ItemMessage:
    {
        quint16 item=0;
        uchar part=0;
        qint8 foe = 0;
        qint16 other=0;
        qint16 berry = 0;
        in >> item >> part >> foe >> berry >> other;
        QString mess = ItemInfo::Message(item, part);
        mess.replace("%st", StatInfo::Stat(other));
        mess.replace("%s", nick(spot));
        mess.replace("%f", nick(foe));
        mess.replace("%i", ItemInfo::Name(berry));
        mess.replace("%m", MoveInfo::Name(other));
        /* Balloon gets a really special treatment */
        if (item == 35)
            printHtml(QString("<b>%1</b>").arg(escapeHtml(tu(mess))));
        else
            printLine(tu(mess));
        break;
    }
    case Flinch:
        printLine(tu(tr("%1 flinched!").arg(nick(spot))));
        break;
    case Recoil:
    {
        bool damage;
        in >> damage;

        if (damage)
            printLine(tu(tr("%1 is hit with recoil!").arg(nick(spot))));
        else
            printLine(tu(tr("%1 had its energy drained!").arg(nick(spot))));
        break;
    }
    case WeatherMessage: {
        qint8 wstatus, weather;
        in >> wstatus >> weather;
        if (weather == NormalWeather)
            break;

        QColor c = Theme::TypeColor(TypeInfo::TypeForWeather(weather));
        switch(wstatus) {
        case EndWeather:
            switch(weather) {
            case Hail: printHtml(toColor(tr("The hail subsided!"),c)); break;
            case SandStorm: printHtml(toColor(tr("The sandstorm subsided!"),c)); break;
            case Sunny: printHtml(toColor(tr("The sunlight faded!"),c)); break;
            case Rain: printHtml(toColor(tr("The rain stopped!"),c)); break;
            } break;
        case HurtWeather:
            switch(weather) {
            case Hail: printHtml(toColor(tr("%1 is buffeted by the hail!").arg(tu(nick(spot))),c)); break;
            case SandStorm: printHtml(toColor(tr("%1 is buffeted by the sandstorm!").arg(tu(nick(spot))),c)); break;
            } break;
        case ContinueWeather:
            switch(weather) {
            case Hail: printHtml(toColor(tr("Hail continues to fall!"),c)); break;
            case SandStorm: printHtml(toColor(tr("The sandstorm rages!"),c)); break;
            case Sunny: printHtml(toColor(tr("The sunlight is strong!"),c)); break;
            case Rain: printHtml(toColor(tr("Rain continues to fall!"),c)); break;
            } break;
        }
    } break;
    case StraightDamage :
    {
        qint16 damage;
        in >> damage;

        printLine(tu(tr("%1 lost %2% of its health!").arg(nick(spot)).arg(damage)));

        break;
    }
    case AbilityMessage:
    {
        quint16 ab=0;
        uchar part=0;
        qint8 type(0), foe(0);
        qint16 other(0);
        in >> ab >> part >> type >> foe >> other;
        QString mess = AbilityInfo::Message(ab,part);
        mess.replace("%st", StatInfo::Stat(other));
        mess.replace("%s", nick(spot));
        //            mess.replace("%ts", name(spot));
        //            mess.replace("%tf", name(!spot));
        mess.replace("%t", TypeInfo::Name(type));
        mess.replace("%f", nick(foe));
        mess.replace("%m", MoveInfo::Name(other));
        //            mess.replace("%d", QString::number(other));
        mess.replace("%i", ItemInfo::Name(other));
        mess.replace("%a", AbilityInfo::Name(other));
        mess.replace("%p", PokemonInfo::Name(other));
        if (type == Pokemon::Normal) {
            printLine(escapeHtml(tu(mess)));
        } else {
            printHtml(toColor(escapeHtml(tu(mess)),Theme::TypeColor(type)));
        }
        break;
    }
    case Substitute:
        in >> info().sub[spot];
        printLine(QString("%1 has a subtitute: %2").arg(nick(spot)).arg(info().sub[spot]), true);
        mydisplay->updatePoke(spot);
        break;
    case BattleEnd:
    {
        printLine("");
        qint8 res;
        in >> res;
        battleEnded = true;
        if (res == Tie) {
            printHtml(toBoldColor(tr("Tie between %1 and %2!").arg(name(info().myself), name(info().opponent)), Qt::blue));
        } else {
            printHtml(toBoldColor(tr("%1 won the battle!").arg(name(spot)), Qt::blue));
        }
        break;
    }
    case BlankMessage:
        printLine("");
        break;
    case Clause:
    {
        printLine(ChallengeInfo::battleText(truespot));
        break;
    }
    case Rated:
    {
        bool rated;
        in >> rated;
        printHtml(toBoldColor(tr("Rule: "), Qt::blue) + (rated? tr("Rated") : tr("Unrated")));

        for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
            if (conf().clauses & (1 << i)) {
                printHtml(toBoldColor(tr("Rule: "), Qt::blue) + ChallengeInfo::clause(i));
            }
        }

        break;
    }
    case TierSection:
    {
        QString tier;
        in >> tier;
        printHtml(toBoldColor(tr("Tier: "), Qt::blue) + tier);
        printHtml(toBoldColor(tr("Mode: "), Qt::blue) + ChallengeInfo::modeName(info().mode));
        break;
    }
    case DynamicInfo:
    {
        in >> info().statChanges[spot];
        mydisplay->updateToolTip(spot);
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
            mydisplay->updatePoke(spot);
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
            mydisplay->updatePoke(spot);
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

        if (info().currentShallow(info().slot(spot, s2)).status() == Pokemon::Koed) {
            printLine(tr("%1 shifted spots to the middle!").arg(tu(nick(info().slot(spot, s1)))), silent);
        } else {
            printLine(tr("%1 shifted spots with %2!").arg(tu(nick(info().slot(spot, s1))), nick(info().slot(spot, s2))), silent);
        }

        info().switchOnSide(spot, s1, s2);

        int pk1 = info().slot(spot, s1);
        int pk2 = info().slot(spot, s2);
        mydisplay->updatePoke(pk1);
        mydisplay->updatePoke(pk2);

        mydisplay->updatePoke(info().player(spot), s1);
        mydisplay->updatePoke(info().player(spot), s2);

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

    mydisplay->updatePoke(spot);
}

void BaseBattleWindow::addSpectator(bool come, int id)
{
    if (come) {
        spectators.insert(id);
    } else {
        spectators.remove(id);
    }
    QString mess = come ? tr("%1 is watching the battle.") : tr("%1 stopped watching the battle.");
    printHtml(toBoldColor(mess.arg(client()->name(id)), Qt::green));
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

BaseBattleDisplay::BaseBattleDisplay(BaseBattleInfo &i)
    : myInfo(&i)
{
    parent = NULL;

    QVBoxLayout *l=  new QVBoxLayout(this);
    l->setMargin(0);

    /* As anyway the BaseGraphicsZone is a fixed size, it's useless to
       resize that part, might as well let  the chat be resized */
    l->setSizeConstraint(QLayout::SetFixedSize);

    nick.resize(i.numberOfSlots);
    level.resize(i.numberOfSlots);
    gender.resize(i.numberOfSlots);
    bars.resize(i.numberOfSlots);
    status.resize(i.numberOfSlots);

    QHBoxLayout *firstLine = new QHBoxLayout();
    l->addLayout(firstLine);

    QLabel* oppPoke = new QLabel();
    oppPoke->setPixmap(Theme::Sprite("hpbar"));
    oppPoke->setFixedSize(oppPoke->pixmap()->size());
    firstLine->addWidget(oppPoke);

    QVBoxLayout *oppl = new QVBoxLayout(oppPoke);
    oppl->setMargin(5);
    oppl->setSpacing(0);
    oppl->addSpacing(3);

    QHBoxLayout *oppl2 = new QHBoxLayout();
    oppl->addLayout(oppl2);
    oppl2->setMargin(0);
    oppl2->setSpacing(6);

    for (int i = 0; i < info().numberOfSlots/2; i++) {
        QGridLayout *inside = new QGridLayout();
        oppl2->addLayout(inside);
        inside->setMargin(0);
        inside->setSpacing(4);

        int slot = info().slot(info().opponent, i);

        nick[slot] = new QLabel();
        nick[slot]->setObjectName("PokemonNick");
        inside->addWidget(nick[slot], 0, 0, 1, 1, Qt::AlignLeft);
        inside->setColumnStretch(0, 100);

        status[slot] = new QLabel();
        inside->addWidget(status[slot], 0, 1);

        gender[slot] = new QLabel();
        inside->addWidget(gender[slot], 0, 2);

        level[slot] = new QLabel();
        level[slot]->setObjectName("PokemonLevel");
        inside->addWidget(level[slot], 0, 3);


        QHBoxLayout *barL = new QHBoxLayout();
        barL->setMargin(0);
        barL->setSpacing(0);
        QLabel *HPIcon = new QLabel();
        HPIcon->setPixmap(Theme::Sprite("hpsquare"));
        HPIcon->setFixedSize(HPIcon->pixmap()->size());
        barL->addWidget(HPIcon);
        bars[slot] = new QClickPBar();
        bars[slot]->setObjectName("LifePoints"); /* for stylesheets */
        bars[slot]->setRange(0, 100);
        barL->addWidget(bars[slot]);

        inside->addLayout(barL,1,0,1,4);
    }

    QHBoxLayout *foeteam = new QHBoxLayout();
    foeteam->addStretch(100);
    for (int i = 0; i < 6; i++) {
        advpokeballs[i] = new QLabel();
        advpokeballs[i]->setPixmap(Theme::StatusIcon(Pokemon::Fine));
        foeteam->addWidget(advpokeballs[i],0,Qt::AlignTop);
    }
    foeteam->setSpacing(1);

    QVBoxLayout * oppTeamAndName = new QVBoxLayout();
    oppTeamAndName->addLayout(foeteam);
    oppTeamAndName->addWidget(trainers[info().opponent] = new QLabel(info().name(info().opponent)),0, Qt::AlignRight);
    trainers[info().opponent]->setObjectName("TrainerNick");
    firstLine->addLayout(oppTeamAndName);


    zone = new BaseGraphicsZone(&info());

    QHBoxLayout *midzone = new QHBoxLayout();
    QVBoxLayout *midme = new QVBoxLayout();
    midzone->addLayout(midme);
    midme->addStretch(100);
    timers[info().myself] = new QProgressBar();
    timers[info().myself]->setObjectName("TimeOut"); //for style sheets
    timers[info().myself]->setRange(0,300);
    midme->addWidget(timers[info().myself]);
    QLabel *mybox = new QLabel();
    mybox->setObjectName("MyTrainerBox");
    mybox->setFixedSize(82,82);
    mybox->setPixmap(Theme::TrainerSprite(info().pInfo[info().myself].avatar));
    midme->addWidget(mybox);
    midzone->addWidget(zone);
    QVBoxLayout *midopp = new QVBoxLayout();
    midzone->addLayout(midopp);
    QLabel *oppbox = new QLabel();
    oppbox->setPixmap(Theme::TrainerSprite(info().pInfo[info().opponent].avatar));
    oppbox->setObjectName("OppTrainerBox");
    oppbox->setFixedSize(82,82);
    midopp->addWidget(oppbox);
    timers[info().opponent] = new QProgressBar();
    timers[info().opponent]->setObjectName("TimeOut"); //for style sheets
    timers[info().opponent]->setRange(0,300);
    midopp->addWidget(timers[info().opponent]);
    midopp->addStretch(100);

    l->addLayout(midzone);


    QHBoxLayout *lastLine = new QHBoxLayout();
    l->addLayout(lastLine);

    QHBoxLayout *team = new QHBoxLayout();
    for (int i = 0; i < 6; i++) {
        mypokeballs[i] = new QLabel();
        mypokeballs[i]->setPixmap(Theme::StatusIcon(Pokemon::Fine));
        team->addWidget(mypokeballs[i],0,Qt::AlignBottom);
    }
    team->setSpacing(1);
    team->addStretch(100);

    QVBoxLayout * myTeamAndName = new QVBoxLayout();
    myTeamAndName->addWidget(trainers[info().myself] = new QLabel(info().name(info().myself)),0, Qt::AlignLeft);
    myTeamAndName->addLayout(team);
    trainers[info().myself]->setObjectName("TrainerNick");

    lastLine->addLayout(myTeamAndName);

    QLabel* myPoke = new QLabel();
    myPoke->setPixmap(Theme::Sprite("hpbar"));
    myPoke->setFixedSize(myPoke->pixmap()->size());
    lastLine->addWidget(myPoke);

    QVBoxLayout *myl = new QVBoxLayout(myPoke);
    myl->setMargin(5);
    myl->setSpacing(0);
    myl->addSpacing(3);

    QHBoxLayout *myl2 = new QHBoxLayout();
    myl->addLayout(myl2);
    myl2->setMargin(0);
    myl2->setSpacing(6);

    for (int i = 0; i < info().numberOfSlots/2; i++) {

        QGridLayout *inside = new QGridLayout();
        myl2->addLayout(inside);
        inside->setMargin(0);
        inside->setSpacing(4);

        int slot = info().slot(info().myself, i);

        nick[slot] = new QLabel();
        nick[slot]->setObjectName("PokemonNick");
        inside->addWidget(nick[slot], 0, 0, 1, 1, Qt::AlignLeft);
        inside->setColumnStretch(0, 100);

        status[slot] = new QLabel();
        inside->addWidget(status[slot], 0, 1);

        gender[slot] = new QLabel();
        inside->addWidget(gender[slot], 0, 2);

        level[slot] = new QLabel();
        level[slot]->setObjectName("PokemonLevel");
        inside->addWidget(level[slot], 0, 3);


        QHBoxLayout *barL = new QHBoxLayout();
        barL->setMargin(0);
        barL->setSpacing(0);
        QLabel *HPIcon = new QLabel();
        HPIcon->setPixmap(Theme::Sprite("hpsquare"));
        HPIcon->setFixedSize(HPIcon->pixmap()->size());
        barL->addWidget(HPIcon);
        bars[slot] = new QClickPBar();
        bars[slot]->setObjectName("LifePoints"); /* for stylesheets */
        bars[slot]->setRange(0, 100);
        barL->addWidget(bars[slot]);

        inside->addLayout(barL,1,0,1,4);
    }

    for (int i = 0; i < info().numberOfSlots; i++) {
        updatePoke(i);
    }

    updateTimers();

    QTimer *t = new QTimer (this);
    t->start(200);
    connect(t, SIGNAL(timeout()), SLOT(updateTimers()));
}

void BaseBattleDisplay::updateTimers()
{
    for (int i = 0; i <= 1; i++) {
        int ctime = std::max(long(0), info().ticking[i] ? info().time[i] + info().startingTime[i] - time(NULL) : info().time[i]);
        if (ctime <= 5*60) {
            timers[i]->setValue(ctime);
        } else {
            timers[i]->setValue(300);
        }
        timers[i]->setFormat(QString("%1 : %2").arg(ctime/60).arg(QString::number(ctime%60).rightJustified(2,'0')));
        if (ctime > 60) {
            timers[i]->setStyleSheet("::chunk{background-color: #55a8fc;}");
        }else if (ctime > 30) {
            timers[i]->setStyleSheet("::chunk{background-color: #F8DB17;;}");
        } else {
            timers[i]->setStyleSheet("::chunk{background-color: #D40202;}");
        }
    }
}

void BaseBattleDisplay::updatePoke(int spot)
{
    if (!parent) {
        parent = dynamic_cast<BaseBattleWindow*>(QWidget::parent());

        if (!parent)
            return;
    }

    if (info().pokeAlive[spot]) {
        const ShallowBattlePoke &poke = info().currentShallow(spot);
        zone->switchTo(poke, spot, info().sub[spot], info().specialSprite[spot]);
        nick[spot]->setText(parent->rnick(spot));
        level[spot]->setText(tr("Lv. %1").arg(poke.level()));
        updateHp(spot);
        bars[spot]->setStyleSheet(health(poke.lifePercent()));
        gender[spot]->setPixmap(Theme::GenderPicture(poke.gender(), Theme::BattleM));
        int status = poke.status();
        this->status[spot]->setPixmap(Theme::BattleStatusIcon(status));

        if (info().player(spot) == info().myself) {
            mypokeballs[info().slotNum(spot)]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
        } else {
            advpokeballs[info().slotNum(spot)]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
        }
    }  else {
        zone->switchToNaught(spot);
        nick[spot]->setText("");
        this->status[spot]->setPixmap(Theme::BattleStatusIcon(Pokemon::Fine));
        gender[spot]->setPixmap(QPixmap());
        bars[spot]->setValue(0);
    }
}

void BaseBattleDisplay::updatePoke(int player, int index)
{
    ShallowBattlePoke &poke = info().pokemons[player][index];

    if (player == info().myself) {
        mypokeballs[index]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
    } else {
        advpokeballs[index]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
    }

    changeStatus(player, index, poke.status());
}

void BaseBattleDisplay::updateHp(int spot)
{
    bars[spot]->setValue(info().currentShallow(spot).lifePercent());
}

void BaseBattleDisplay::updateToolTip(int spot)
{
    if (!parent) {
        parent = dynamic_cast<BaseBattleWindow*>(QWidget::parent());

        if (!parent)
            return;
    }
    QString tooltip;

    QString stats[7] = {
        tu(StatInfo::Stat(1)),
        tu(StatInfo::Stat(2)),
        tu(StatInfo::Stat(3)),
        tu(StatInfo::Stat(4)),
        tu(StatInfo::Stat(5)),
        tu(StatInfo::Stat(6)),
        tu(StatInfo::Stat(7))
    };
    int max = 0;
    for (int i = 0; i < 7; i++) {
        max = std::max(max, stats[i].length());
    }
    for (int i = 0; i < 7; i++) {
        stats[i] = stats[i].leftJustified(max, '.', false);
    }

    const ShallowBattlePoke &poke = info().currentShallow(spot);

    tooltip += parent->rnick(spot) + "\n";
    tooltip += TypeInfo::Name(PokemonInfo::Type1(poke.num(), info().gen));
    int type2 = PokemonInfo::Type2(poke.num());
    if (type2 != Pokemon::Curse) {
        tooltip += " " + TypeInfo::Name(PokemonInfo::Type2(poke.num(), info().gen));
    }
    tooltip += "\n";

    for (int i = 0; i < 5; i++) {
        tooltip += "\n" + stats[i] + " ";
        int boost = info().statChanges[spot].boosts[i];
        if (boost >= 0) {
            tooltip += QString("+%1").arg(boost);
        } else if (boost < 0) {
            tooltip += QString("%1").arg(boost);
        }
    }
    for (int i = 5; i < 7; i++) {
        int boost = info().statChanges[spot].boosts[i];
        if (boost) {
            tooltip += "\n" + stats[i] + " ";

            if (boost > 0) {
                tooltip += QString("+%1").arg(boost);
            } else if (boost < 0) {
                tooltip += QString("%1").arg(boost);
            }
        }
    }

    tooltip += "\n";

    int flags = info().statChanges[spot].flags;

    int spikes[3] = {BattleDynamicInfo::Spikes, BattleDynamicInfo::SpikesLV2 ,BattleDynamicInfo::SpikesLV3};
    for (int i = 0; i < 3; i++) {
        if (flags & spikes[i]) {
            tooltip += "\n" + tr("Spikes level %1").arg(i+1);
            break;
        }
    }

    int tspikes[2] = {BattleDynamicInfo::ToxicSpikes, BattleDynamicInfo::ToxicSpikesLV2};
    for (int i = 0; i < 2; i++) {
        if (flags & tspikes[i]) {
            tooltip += "\n" + tr("Toxic Spikes level %1").arg(i+1);
            break;
        }
    }

    if (flags & BattleDynamicInfo::StealthRock) {
        tooltip += "\n" + tr("Stealth Rock");
    }

    zone->tooltips[spot] = tooltip;
}

void BaseBattleDisplay::changeStatus(int spot, int poke, int status) {
    if (info().player(spot)==info().myself) {
        mypokeballs[poke]->setPixmap(Theme::StatusIcon(status));
    } else {
        advpokeballs[poke]->setPixmap(Theme::StatusIcon(status));
    }
}

QString BaseBattleDisplay::health(int lifePercent)
{
    return lifePercent > 50 ? "::chunk{background-color: #1fc42a;}" : (lifePercent >= 26 ? "::chunk{background-color: #F8DB17;}" : "::chunk{background-color: #D40202;}");
}

BaseGraphicsZone::BaseGraphicsZone(BaseBattleInfo *i) : mInfo(i)
{
    setScene(&scene);
    setMouseTracking(true);

    tooltips.resize(info().numberOfSlots);
    items.resize(info().numberOfSlots);

    scene.setSceneRect(0,0,257,145);
    scene.addItem(new QGraphicsPixmapItem(Theme::Pic(QString("battle_fields/%1.png").arg((rand()%11)+1))));

    for (int i = 0; i < info().numberOfSlots; i++) {
        items[i] = new QGraphicsPixmapItem();
        scene.addItem(items[i]);
    }

    int size = Version::avatarSize[info().gen-1];

    if (!info().multiples()) {
        items[info().slot(info().myself)]->setPos(50 - size/2, 146 - size);
        items[info().slot(info().opponent)]->setPos(184- size/2, 96 - size);
    } else {
        for (int i = 0; i < info().numberOfSlots/2; i++) {
            items[info().slot(info().myself, i)]->setPos(i*60, 146-size);
            int base = 257-80-(info().numberOfSlots/2 - 1)*60;
            items[info().slot(info().opponent, i)]->setPos(base+i*60, 96 - size);
        }
    }
}

void BaseGraphicsZone::updatePos(int spot)
{
    int player = info().player(spot);

    int width = items[spot]->pixmap().width();
    int height = items[spot]->pixmap().height();

    if (player == info().myself) {
        if (!info().multiples()) {
            items[spot]->setPos(50 - width/2, 146 - height);
        } else {
            items[spot]->setPos(info().slotNum(spot)*60, 146-height);
        }
    } else {
        if (!info().multiples()) {
            items[spot]->setPos(184 - width/2, 96 - height);
        } else {
            int base = 257-80-(info().numberOfSlots/2 - 1)*60;
            items[spot]->setPos(base + info().slotNum(spot)*60, 96-height);
        }
    }
}

void BaseGraphicsZone::switchToNaught(int spot)
{
    items[spot]->setPixmap(QPixmap());
}

QPixmap BaseGraphicsZone::loadPixmap(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub)
{
    quint64 key = this->key(num, shiny, back, gender, sub);

    if (!graphics.contains(key)) {
        QPixmap p;
        if (sub) {
            p = PokemonInfo::Sub(info().gen, back);
        } else {
            p = PokemonInfo::Picture(num, info().gen, gender, shiny, back);
        }

        QImage img = p.toImage();
        cropImage(img);
        p = QPixmap::fromImage(img);

        graphics.insert(key, p);
    }

    return graphics[key];
}

quint64 BaseGraphicsZone::key(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub) const
{
    return sub ? ((1 << 27) + (back << 28)) : (num.pokenum + (num.subnum << 16) + (gender << 24) + (back << 26) + (shiny<<27));
}

void BaseGraphicsZone::mouseMoveEvent(QMouseEvent * e)
{
    QGraphicsItem *it = this->itemAt(e->pos());

    for (int i = 0; i < items.size(); i++) {
        if (items[i] == it) {
            QToolTip::setFont(QFont("Courier New",8));
            QToolTip::showText(e->globalPos(), tooltips[i]);
            break;
        }
    }
}
