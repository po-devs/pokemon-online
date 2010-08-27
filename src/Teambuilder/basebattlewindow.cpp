#include "basebattlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"

BaseBattleInfo::BaseBattleInfo(const PlayerInfo &me, const PlayerInfo &opp, bool doubles, int myself, int opponent)
    : myself(myself), opponent(opponent)
{
    this->doubles=  doubles;
    this->numberOfSlots = doubles?4:2;

    for (int i = 0; i < numberOfSlots; i++) {
        sub.push_back(false);
        pokeAlive.push_back(false);
        specialSprite.push_back(0);
        lastSeenSpecialSprite.push_back(0);
        currentIndex.push_back(i/2);
        statChanges.push_back(BattleDynamicInfo());
    }

    pInfo[myself] = me;
    pInfo[opponent] = opp;

    time[myself] = 5*60;
    time[opponent] = 5*60;
    ticking[myself] = false;
    ticking[opponent] = false;
}

BaseBattleWindow::BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf) :
        delayed(0), ignoreSpecs(false)
{
    this->conf() = conf;
    myInfo = new BaseBattleInfo(me, opponent, conf.doubles);
    info().gen = conf.gen;
    mydisplay = new BaseBattleDisplay(info());

    init();
    show();
    printHtml(toBoldColor(tr("Battle between %1 and %2 is underway!"), Qt::blue).arg(name(true), name(false)));
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
    chat->addWidget(mychat = new QScrollDownTextEdit());
    mychat->setAutoClear(false);
    chat->addWidget(myline = new QIRCLineEdit());
    QHBoxLayout * buttons = new QHBoxLayout();
    chat->addLayout(buttons);
    QPushButton *myignore;
    buttons->addWidget(mysend = new QPushButton(tr("C&hat")));
    buttons->addWidget(myignore = new QPushButton(tr("&Ignore Spectators")));
    myignore->setCheckable(true);

    connect(musicOn, SIGNAL(toggled(bool)), SLOT(musicPlayStop()));
    connect(myignore, SIGNAL(toggled(bool)), SLOT(ignoreSpectators(bool)));
    connect(myclose, SIGNAL(clicked()), SLOT(clickClose()));
    connect(myline, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(mysend, SIGNAL(clicked()), SLOT(sendMessage()));

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
    if (saveLogs->isChecked()) {
        QSettings s;

        QString n1(info().pInfo[0].team.name), n2(info().pInfo[1].team.name);

        /* Those characters are banned in file names on windows */
        QList<QChar> bannedCh = QList<QChar> () << '"' << '/' << '\\' << ':' << '*' << '|' << '?' << '<' << '>';
        foreach(QChar c, bannedCh) {
            n1 = n1.replace(c, ' ');
            n2 = n2.replace(c, ' ');
        }

        if(!QDir::home().exists(s.value("battle_logs_directory").toString())) {
            QDir::home().mkpath(s.value("battle_logs_directory").toString());
        }
        QString file = s.value("battle_logs_directory").toString() + n1 + " vs " + n2 + "--" + QDate::currentDate().toString("dd MMMM yyyy")
               + " at " +QTime::currentTime().toString("hh'h'mm") + ".html";
        QFile out (file);
        out.open(QIODevice::WriteOnly);
        out.write(mychat->toHtml().toUtf8());
    }
}

void BaseBattleWindow::closeEvent(QCloseEvent *)
{
    checkAndSaveLog();
    emit closedBW(battleId());
    close();
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
        myline->myclear();
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

void BaseBattleWindow::ignoreSpectators(bool ignore)
{
    ignoreSpecs = ignore;
}

void BaseBattleWindow::dealWithCommandInfo(QDataStream &in, int command, int spot, int truespot)
{
    switch (command)
    {
    case SendOut:
        {
            bool silent;
            in >> silent;
            in >> info().currentIndex[spot];
            in >> info().currentShallow(spot);
            info().pokeAlive[spot] = true;
            info().sub[spot] = false;
            info().specialSprite[spot] = 0;
            mydisplay->updatePoke(spot);

            //Plays the battle cry when a pokemon is switched in
            if (musicPlayed())
            {
                playCry(info().currentShallow(spot).num());
            }


            if (!silent) {
                QString pokename = PokemonInfo::Name(info().currentShallow(spot).num());
                if (pokename != rnick(spot))
                    printLine(tr("%1 sent out %2! (%3)").arg(name(player(spot)), rnick(spot), pokename));
                else
                    printLine(tr("%1 sent out %2!").arg(name(player(spot)), rnick(spot)));
            }

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

            printHtml(tr("%1 used %2!").arg(escapeHtml(tu(nick(spot))), toBoldColor(MoveInfo::Name(attack), TypeInfo::Color(MoveInfo::Type(attack)))));
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

            animatedHpSpot() = spot;
            animatedHpGoal() = newHp;
            animateHPBar();
            break;
        }
    case Ko:
        //Plays the battle cry when a pokemon faints
        if (musicPlayed())
        {
            playCry(info().currentShallow(spot).num());
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
                tr("%1 was burned!"),
                tr("%1 was frozen solid!"),
                tr("%1 fell asleep!"),
                tr("%1 was poisoned!"),
                tr("%1 was badly poisoned!")
            };

            qint8 status;
            in >> status;
            if (status > 0) {
                printHtml(toColor(tu(statusChangeMessages[status-1].arg(nick(spot))), StatInfo::StatusColor(status)));
            } else if (status == -1) {
                printHtml(toColor(escapeHtml(tu(tr("%1 became confused!").arg(nick(spot)))), TypeInfo::Color(Type::Ghost).name()));
            }
            break;
        }
    case AbsStatusChange:
        {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        if (status != -1) {
            info().currentShallow(spot).status() = status;
            if (poke == info().currentIndex[spot])
                mydisplay->updatePoke(spot);
        }
        mydisplay->changeStatus(spot,poke,status);
        break;
    }
    case AlreadyStatusMessage:
        printHtml(toColor(tr("%1 is already %2.").arg(tu(nick(spot)), StatInfo::Status(info().currentShallow(spot).status())),
                          StatInfo::StatusColor(info().currentShallow(spot).status())));
        break;
    case StatusMessage:
        {
            qint8 status;
            in >> status;
            switch(status)
            {
     case FeelConfusion:
                printHtml(toColor(escapeHtml(tu(tr("%1 is confused!").arg(nick(spot)))), TypeInfo::Color(Type::Ghost).name()));
                break;
     case HurtConfusion:
                printHtml(toColor(escapeHtml(tu(tr("It hurt itself in its confusion!"))), TypeInfo::Color(Type::Ghost).name()));
                break;
     case FreeConfusion:
                printHtml(toColor(escapeHtml(tu(tr("%1 snapped out its confusion!").arg(nick(spot)))), TypeInfo::Color(Type::Dark).name()));
                break;
     case PrevParalysed:
                printHtml(toColor(escapeHtml(tu(tr("%1 is paralyzed! It can't move!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Paralysed)));
                break;
     case FeelAsleep:
                printHtml(toColor(escapeHtml(tu(tr("%1 is fast asleep!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Asleep)));
                break;
     case FreeAsleep:
                printHtml(toColor(escapeHtml(tu(tr("%1 woke up!").arg(nick(spot)))), TypeInfo::Color(Type::Dark).name()));
                break;
     case HurtBurn:
                printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by its burn!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Burnt)));
                break;
     case HurtPoison:
                printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by poison!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Poisoned)));
                break;
     case PrevFrozen:
                printHtml(toColor(escapeHtml(tu(tr("%1 is frozen solid!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Frozen)));
                break;
     case FreeFrozen:
                printHtml(toColor(escapeHtml(tu(tr("%1 thawed out!").arg(nick(spot)))), TypeInfo::Color(Type::Dark).name()));
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
            if (ignoreSpecs)
                return;
            qint32 id;
            QString message;
            in >> id >> message;
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
            printHtml(toColor(escapeHtml(tu(mess)), TypeInfo::Color(type)));
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
            printLine(tu(mess));
            break;
        }
    case Flinch:
        printLine(tu(tr("%1 flinched!").arg(nick(spot))));
        break;
    case Recoil:
        printLine(tu(tr("%1 is hit with recoil!").arg(nick(spot))));
        break;
    case WeatherMessage: {
            qint8 wstatus, weather;
            in >> wstatus >> weather;
            if (weather == NormalWeather)
                break;
            QColor c = (weather == Hail ? TypeInfo::Color(Type::Ice) : (weather == Sunny ? TypeInfo::Color(Type::Fire) : (weather == SandStorm ? TypeInfo::Color(Type::Rock) : TypeInfo::Color(Type::Water))));
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
        mess.replace("%s", nick(spot));
        //            mess.replace("%ts", name(spot));
        //            mess.replace("%tf", name(!spot));
        mess.replace("%t", TypeInfo::Name(type));
        mess.replace("%f", nick(foe));
        mess.replace("%m", MoveInfo::Name(other));
        //            mess.replace("%d", QString::number(other));
        mess.replace("%i", ItemInfo::Name(other));
        mess.replace("%a", AbilityInfo::Name(other));
        //            mess.replace("%p", PokemonInfo::Name(other));
        if (type == Pokemon::Normal) {
            printLine(escapeHtml(tu(mess)));
        } else {
            printHtml(toColor(escapeHtml(tu(mess)),TypeInfo::Color(type)));
        }
        break;
    }
    case Substitute:
        in >> info().sub[spot];
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
            bool inBattle;
            in >> inBattle;

            if (inBattle) {
                printLine(ChallengeInfo::battleText(truespot));
            } else {
                printHtml(toBoldColor(tr("Rule: "), Qt::blue) + ChallengeInfo::clause(truespot));
            }
            break;
        }
    case Rated:
        {
            bool rated;
            in >> rated;
            printHtml(toBoldColor(tr("Rule: "), Qt::blue) + (rated? tr("Rated") : tr("Unrated")));
            break;
        }
    case TierSection:
        {
            QString tier;
            in >> tier;
            printHtml(toBoldColor(tr("Tier: "), Qt::blue) + tier);
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
                int old = info().specialSprite[spot];
                in >> info().specialSprite[spot];
                if (info().specialSprite[spot] == -1) {
                    info().lastSeenSpecialSprite[spot] = old;
                } else if (info().specialSprite[spot] == 0) {
                    info().specialSprite[spot] = info().lastSeenSpecialSprite[spot];
                }
                mydisplay->updatePoke(spot);
            } else if (type == DefiniteForm)
            {
                quint8 poke;
                quint16 newform;
                in >> poke >> newform;
                info().pokemons[spot][poke].num() = newform;
                if (poke == info().currentIndex[spot]) {
                    info().currentShallow(spot).num() = newform;
                }
            } else if (type == AestheticForme)
            {
                quint8 newforme;
                in >> newforme;
                info().currentShallow(spot).forme() = newforme;
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

void BaseBattleWindow::printLine(const QString &str)
{
    if (str == "" && blankMessage) {
        return;
    }

    if (str == "") {
        blankMessage = true;
        mychat->insertHtml("");
    } else {
        blankMessage = false;
    }

    mychat->insertHtml(str + "<br />");
}

void BaseBattleWindow::printHtml(const QString &str)
{
    blankMessage = false;
    mychat->insertHtml(str + "<br />");
}

BaseBattleDisplay::BaseBattleDisplay(BaseBattleInfo &i)
    : myInfo(&i)
{
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
    oppPoke->setPixmap(QPixmap("db/BattleWindow/NewPokeBar.png"));
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
        HPIcon->setPixmap(QPixmap("db/BattleWindow/HP.png"));
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
        advpokeballs[i]->setPixmap(StatInfo::Icon(Pokemon::Fine));
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
    mybox->setPixmap(QPixmap(QString("db/Trainer Sprites/%1.png").arg(info().pInfo[info().myself].avatar)));
    midme->addWidget(mybox);
    midzone->addWidget(zone);
    QVBoxLayout *midopp = new QVBoxLayout();
    midzone->addLayout(midopp);
    QLabel *oppbox = new QLabel();
    oppbox->setPixmap(QPixmap(QString("db/Trainer Sprites/%1.png").arg(info().pInfo[info().opponent].avatar)));
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
        mypokeballs[i]->setPixmap(StatInfo::Icon(Pokemon::Fine));
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
    myPoke->setPixmap(QPixmap("db/BattleWindow/NewPokeBar.png"));
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
        HPIcon->setPixmap(QPixmap("db/BattleWindow/HP.png"));
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
    if (info().pokeAlive[spot]) {
        const ShallowBattlePoke &poke = info().currentShallow(spot);
        zone->switchTo(poke, spot, info().sub[spot], info().specialSprite[spot]);
        nick[spot]->setText(poke.nick());
        level[spot]->setText(tr("Lv. %1").arg(poke.level()));
        updateHp(spot);
        bars[spot]->setStyleSheet(health(poke.lifePercent()));
        gender[spot]->setPixmap(GenderInfo::Picture(poke.gender(), true));
        int status = poke.status();
        this->status[spot]->setPixmap(StatInfo::BattleIcon(status));

        if (info().player(spot) == info().myself) {
            mypokeballs[info().currentIndex[spot]]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
        } else {
            advpokeballs[info().currentIndex[spot]]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
        }
    }  else {
        zone->switchToNaught(spot);
        nick[spot]->setText("");
        this->status[spot]->setPixmap(StatInfo::BattleIcon(Pokemon::Fine));
        gender[spot]->setPixmap(QPixmap());
        bars[spot]->setValue(0);
    }
}

void BaseBattleDisplay::updateHp(int spot)
{
    bars[spot]->setValue(info().currentShallow(spot).lifePercent());
}

void BaseBattleDisplay::updateToolTip(int spot)
{
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

    tooltip += poke.nick() + "\n";
    tooltip += TypeInfo::Name(PokemonInfo::Type1(poke.num()));
    int type2 = PokemonInfo::Type2(poke.num());
    if (type2 != Pokemon::Curse) {
        tooltip += " " + TypeInfo::Name(PokemonInfo::Type2(poke.num()));
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
        mypokeballs[poke]->setPixmap(StatInfo::Icon(status));
    } else {
        advpokeballs[poke]->setPixmap(StatInfo::Icon(status));
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
    scene.addItem(new QGraphicsPixmapItem(QPixmap(QString("db/battle_fields/%1.png").arg((rand()%11)+1))));

    for (int i = 0; i < info().numberOfSlots; i++) {
        items[i] = new QGraphicsPixmapItem();
        scene.addItem(items[i]);
    }

    if (info().gen >= 4) {
        if (!info().doubles) {
            items[info().slot(info().myself)]->setPos(10, 145-79);
            items[info().slot(info().opponent)]->setPos(257-105, 95 - 79);
        } else {
            items[info().slot(info().myself)]->setPos(0, 145-79);
            items[info().slot(info().opponent)]->setPos(257-140, 95 - 79);
            items[info().slot(info().myself,1)]->setPos(80, 145-79);
            items[info().slot(info().opponent,1)]->setPos(257-80, 95 - 79);
        }
    } else if (info().gen <= 3) {
        if (!info().doubles) {
            items[info().slot(info().myself)]->setPos(10+8, 145-63);
            items[info().slot(info().opponent)]->setPos(257-105+8, 95 - 63);
        } else {
            items[info().slot(info().myself)]->setPos(0, 145-63);
            items[info().slot(info().opponent)]->setPos(257-140, 95 - 63);
            items[info().slot(info().myself,1)]->setPos(80, 145-63);
            items[info().slot(info().opponent,1)]->setPos(257-80, 95 - 63);
        }
    }
}

void BaseGraphicsZone::switchToNaught(int spot)
{
    items[spot]->setPixmap(QPixmap());
}

QPixmap BaseGraphicsZone::loadPixmap(quint16 num, quint8 forme, bool shiny, bool back, quint8 gender, bool sub)
{
    qint32 key = this->key(num, forme, shiny, back, gender, sub);

    if (!graphics.contains(key)) {
        if (sub) {
            graphics.insert(key, PokemonInfo::Sub(info().gen, back));
        } else {
            graphics.insert(key, PokemonInfo::Picture(num, info().gen, forme, gender, shiny, back));
        }
    }

    return graphics[key];
}

qint32 BaseGraphicsZone::key(quint16 num, quint8 forme, bool shiny, bool back, quint8 gender, bool sub) const
{
    return sub ? ((1 << 31) + (back << 30)) : (num + (gender << 16) + (back << 18) + (shiny<<19) + (forme << 20));
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

