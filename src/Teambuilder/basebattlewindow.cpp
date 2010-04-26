#include "basebattlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"

BaseBattleInfo::BaseBattleInfo(const PlayerInfo &me, const PlayerInfo &opp)
{
    pInfo[0] = me;
    pInfo[1] = opp;
    sub[0] = false;
    sub[1] = false;
    pokeAlive[0] = false;
    pokeAlive[1] = false;
    specialSprite[0] = 0;
    specialSprite[1] = 0;
    time[0] = 5*60;
    time[1] = 5*60;
    ticking[0] = false;
    ticking[1] = false;
    currentIndex[0] = 0;
    currentIndex[1] = 0;
}

BaseBattleWindow::BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent) : ignoreSpecs(false), delayed(false)
{
    myInfo = new BaseBattleInfo(me, opponent);
    mydisplay = new BaseBattleDisplay(info());
    init();
    show();
    printHtml(toBoldColor(tr("Battle between %1 and %2 is underway!"), Qt::blue).arg(name(true), name(false)));
}

void BaseBattleWindow::delay(int msec)
{
    delayed = true;

    if (msec != 0)
        QTimer::singleShot(msec, this, SLOT(undelay()));
}

void BaseBattleWindow::undelay()
{
    delayed = false;

    while (delayed == false && delayedCommands.size() > 0) {
        receiveInfo(delayedCommands.front());
        delayedCommands.pop_front();
    }
}

void BaseBattleWindow::init()
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    QToolTip::setFont(QFont("Verdana",10));

    blankMessage = false;
    battleEnded = false;

    setWindowTitle(tr("Battle between %1 and %2").arg(name(0), name(1)));

    QHBoxLayout *columns = new QHBoxLayout(this);
    columns->addLayout(mylayout = new QGridLayout());

    mylayout->addWidget(mydisplay, 0, 0, 1, 3);
    mylayout->addWidget(myclose = new QPushButton(tr("&Close")),1,2);

    QVBoxLayout *chat = new QVBoxLayout();
    columns->addLayout(chat);
    chat->addWidget(mychat = new QScrollDownTextEdit());
    mychat->setAutoClear(false);
    chat->addWidget(myline = new QLineEdit());
    QHBoxLayout * buttons = new QHBoxLayout();
    chat->addLayout(buttons);
    QPushButton *myignore;
    buttons->addWidget(mysend = new QPushButton(tr("C&hat")));
    buttons->addWidget(myignore = new QPushButton(tr("&Ignore Spectators")));
    myignore->setCheckable(true);


    connect(myignore, SIGNAL(toggled(bool)), SLOT(ignoreSpectators(bool)));
    connect(myclose, SIGNAL(clicked()), SLOT(clickClose()));
    connect(myline, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(mysend, SIGNAL(clicked()), SLOT(sendMessage()));


    //starts battle music
    musicOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    music = new Phonon::MediaObject(this);
    Phonon::createPath(music, musicOutput);

    QDir directory = QDir("Music");
    QStringList files;
    files = directory.entryList(QStringList("*"), QDir::Files | QDir::NoSymLinks);
    music->setCurrentSource(QString("Music/" + files[rand() % files.size()]));

    QSettings s;
    if (s.value("play_music").toBool())
    {
        musicPlayed() = true;
        music->play();
        //playback
        connect(music, SIGNAL(finished()), music, SLOT(play()));
    } else {
        musicPlayed() = false;
    }

    //layout()->setSizeConstraint(QLayout::SetFixedSize);
}

QString BaseBattleWindow::name(int spot) const
{
    return info().name(spot);
}

QString BaseBattleWindow::nick(int player) const
{
    return tr("%1's %2").arg(name(player), rnick(player));
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
    delay();


    /* We deal with % hp, 30 msecs per % */
    int life = info().currentShallow(spot).lifePercent();

    if (goal == life) {
        delay(120);
        return;
    }

    info().currentShallow(spot).lifePercent() = life < goal ? life+1 : life-1;
    //Recursive call to update the hp bar 30msecs later
    QTimer::singleShot(30, this, SLOT(animateHPBar()));

    mydisplay->updatePoke(spot);
}

void BaseBattleWindow::closeEvent(QCloseEvent *)
{
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
        emit battleMessage(message, battleId());
        myline->clear();
    }
}

void BaseBattleWindow::receiveInfo(QByteArray inf)
{
    if (delayed && inf[0] != char(BattleChat) && inf[0] != char(SpectatorChat)) {
        delayedCommands.push_back(inf);
        return;
    }

    QDataStream in (&inf, QIODevice::ReadOnly);

    uchar command;
    qint8 player;

    in >> command >> player;

    dealWithCommandInfo(in, command, player, player);
}

void BaseBattleWindow::ignoreSpectators(bool ignore)
{
    ignoreSpecs = ignore;
}

void BaseBattleWindow::playMusic(bool play)
{
    if (musicPlayed() == play)
        return;

    musicPlayed() = play;

    if (play) {
     music->play();
    }
    else {
       music->pause();;
    }
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
                    printLine(tr("%1 sent out %2! (%3)").arg(name(spot), rnick(spot), pokename));
                else
                    printLine(tr("%1 sent out %2!").arg(name(spot), rnick(spot)));
            }

            break;
        }
    case SendBack:
        printLine(tr("%1 called %2 back!").arg(name(spot), rnick(spot)));
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
        printLine(tr("Hit!"));
        break;
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
        printHtml(toColor(tr("A critical hit!"), Qt::red));
        break;
    case Miss:
        printLine(tr("The attack of %1 missed!").arg(nick(spot)));
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
        printHtml(toColor(tr("%1 is already %2!").arg(tu(nick(spot)), StatInfo::Status(info().currentShallow(spot).status())),
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
        {
            QString message;
            in >> message;
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
            printHtml(toBoldColor(client()->name(id), Qt::blue) + ": " + escapeHtml(message));
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
            mess.replace("%ts", name(spot));
            mess.replace("%tf", name(!spot));
            mess.replace("%t", TypeInfo::Name(type));
            mess.replace("%f", nick(!spot));
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
        printLine(tr("But there is no target pokémon!"));
        break;
    case ItemMessage:
        {
            quint16 item=0;
            uchar part=0;
            qint8 foe = 0;
            qint8 other=0;
            qint16 berry = 0;
            in >> item >> part >> foe >> berry >> other;
            QString mess = ItemInfo::Message(item, part);
            mess.replace("%st", StatInfo::Stat(other));
            mess.replace("%s", nick(spot));
            mess.replace("%f", nick(!spot));
            mess.replace("%i", ItemInfo::Name(berry));
            mess.replace("%m", MoveInfo::Name(berry));
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
        mess.replace("%f", nick(!spot));
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
                printHtml(toBoldColor(tr("Tie between %1 and %2!").arg(name(Myself), name(Opponent)), Qt::blue));
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
                in >> info().specialSprite[spot];
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

void BaseBattleWindow::playCry(int pokenum)
{
    if (cry) {
        cry->stop();
        cry->deleteLater();
    }

    if (cryOutput) {
        cryOutput->deleteLater();
    }

    cryOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    cry = new Phonon::MediaObject(this);
    Phonon::createPath(cry, cryOutput);

    cryData = PokemonInfo::Cry(pokenum);
    cryBuffer.setBuffer(&cryData);
    cryBuffer.open(QIODevice::ReadOnly);

    cry->setCurrentSource(&cryBuffer);

    cry->play();

    if (cry->isValid()) {
        delay();
        connect(cry, SIGNAL(finished()), SLOT(undelay()));
    }
}

BaseBattleDisplay::BaseBattleDisplay(BaseBattleInfo &i)
    : myInfo(&i)
{
    QVBoxLayout *l=  new QVBoxLayout(this);
    l->setMargin(0);

    /* As anyway the BaseGraphicsZone is a fixed size, it's useless to
       resize that part, might as well let  the chat be resized */
    l->setSizeConstraint(QLayout::SetFixedSize);

    QHBoxLayout *firstLine = new QHBoxLayout();
    l->addLayout(firstLine);

    firstLine->addSpacing(90);
    QLabel* oppPoke = new QLabel();
    oppPoke->setPixmap(QPixmap("db/BattleWindow/OpponentPokeBar.png"));

    nick[Opponent] = new QLabel(oppPoke);
    nick[Opponent]->setObjectName("PokemonNick");
    QFont f("Candara.ttf");
    nick[Opponent]->setFont(f);
    nick[Opponent]->setGeometry(8,7,102,21);
    nick[Opponent]->setAlignment(Qt::AlignVCenter);

    level[Opponent] = new QLabel(oppPoke);
    level[Opponent]->setObjectName("PokemonLevel");
    level[Opponent]->setGeometry(120,11,50,13);

    gender[Opponent] = new QLabel(oppPoke);
    gender[Opponent]->setGeometry(104,11,12,12);

    status[Opponent] = new QLabel(oppPoke);
    status[Opponent]->setGeometry(10,23,23,23);

    bars[Opponent] = new QClickPBar();
    bars[Opponent]->setParent(oppPoke);
    bars[Opponent]->setObjectName("LifePoints"); /* for stylesheets */
    bars[Opponent]->setRange(0, 100);
    bars[Opponent]->setGeometry(75,29,90,15);
    firstLine->addWidget(oppPoke);


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
    oppTeamAndName->addWidget(trainers[Opponent] = new QLabel(info().name(Opponent)),0, Qt::AlignRight);
    trainers[Opponent]->setObjectName("TrainerNick");
    firstLine->addLayout(oppTeamAndName);


    zone = new BaseGraphicsZone();

    QHBoxLayout *midzone = new QHBoxLayout();
    QVBoxLayout *midme = new QVBoxLayout();
    midzone->addLayout(midme);
    midme->addStretch(100);
    timers[Myself] = new QProgressBar();
    timers[Myself]->setObjectName("TimeOut"); //for style sheets
    timers[Myself]->setRange(0,300);
    midme->addWidget(timers[Myself]);
    QLabel *mybox = new QLabel();
    mybox->setObjectName("MyTrainerBox");
    mybox->setFixedSize(82,82);
    mybox->setPixmap(QPixmap(QString("db/Trainer Sprites/%1.png").arg(info().pInfo[Myself].avatar)));
    midme->addWidget(mybox);
    midzone->addWidget(zone);
    QVBoxLayout *midopp = new QVBoxLayout();
    midzone->addLayout(midopp);
    QLabel *oppbox = new QLabel();
    oppbox->setPixmap(QPixmap(QString("db/Trainer Sprites/%1.png").arg(info().pInfo[Opponent].avatar)));
    oppbox->setObjectName("OppTrainerBox");
    oppbox->setFixedSize(82,82);
    midopp->addWidget(oppbox);
    timers[Opponent] = new QProgressBar();
    timers[Opponent]->setObjectName("TimeOut"); //for style sheets
    timers[Opponent]->setRange(0,300);
    midopp->addWidget(timers[Opponent]);
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
    myTeamAndName->addWidget(trainers[Myself] = new QLabel(info().name(Myself)),0, Qt::AlignLeft);
    myTeamAndName->addLayout(team);
    trainers[Myself]->setObjectName("TrainerNick");

    lastLine->addLayout(myTeamAndName);

    QLabel* myPoke = new QLabel();
    myPoke->setPixmap(QPixmap("db/BattleWindow/ChallengerPokeBar.png"));

    nick[Myself] = new QLabel(myPoke);
    nick[Myself]->setObjectName("PokemonNick");
    nick[Myself]->setGeometry(25,7,102,21);
    nick[Myself]->setAlignment(Qt::AlignVCenter);

    level[Myself] = new QLabel(myPoke);
    level[Myself]->setObjectName("PokemonLevel");
    level[Myself]->setGeometry(126,11,50,13);

    gender[Myself] = new QLabel(myPoke);
    gender[Myself]->setGeometry(111,11,12,12);

    status[Myself] = new QLabel(myPoke);
    status[Myself]->setGeometry(14,23,23,23);

    bars[Myself] = new QClickPBar();
    bars[Myself]->setParent(myPoke);
    bars[Myself]->setObjectName("LifePoints"); /* for stylesheets */
    bars[Myself]->setRange(0, 100);
    bars[Myself]->setGeometry(75,29,90,15);

    lastLine->addWidget(myPoke);
    lastLine->addSpacing(90);

    updatePoke(Myself);
    updatePoke(Opponent);
    updateTimers();

    QTimer *t = new QTimer (this);
    t->start(200);
    connect(t, SIGNAL(timeout()), SLOT(updateTimers()));
}

void BaseBattleDisplay::updateTimers()
{
    for (int i = Myself; i <= Opponent; i++) {
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

        if (spot == Myself) {
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
    if (spot == Myself) {
        mypokeballs[poke]->setPixmap(StatInfo::Icon(status));
    } else {
        advpokeballs[poke]->setPixmap(StatInfo::Icon(status));
    }
}

QString BaseBattleDisplay::health(int lifePercent)
{
    return lifePercent > 50 ? "::chunk{background-color: #1fc42a;}" : (lifePercent >= 26 ? "::chunk{background-color: #F8DB17;}" : "::chunk{background-color: #D40202;}");
}

BaseGraphicsZone::BaseGraphicsZone()
{
    setScene(&scene);

    scene.setSceneRect(0,0,257,145);
    scene.addItem(new QGraphicsPixmapItem(QPixmap(QString("db/battle_fields/%1.png").arg((rand()%11)+1))));

    mine = new QGraphicsPixmapItem();
    foe = new QGraphicsPixmapItem();

    scene.addItem(mine);
    mine->setPos(10, 145-79);

    scene.addItem(foe);
    foe->setPos(257-105, 16);
}

void BaseGraphicsZone::switchToNaught(int spot)
{
    if (spot == Myself)
        mine->setPixmap(QPixmap());
    else
        foe->setPixmap(QPixmap());
}

QPixmap BaseGraphicsZone::loadPixmap(quint16 num, quint8 forme, bool shiny, bool back, quint8 gender, bool sub)
{
    qint32 key = this->key(num, forme, shiny, back, gender, sub);

    if (!graphics.contains(key)) {
        if (sub) {
            graphics.insert(key, PokemonInfo::Sub(back));
        } else {
            graphics.insert(key, PokemonInfo::Picture(num, forme, gender, shiny, back));
        }
    }

    return graphics[key];
}

qint32 BaseGraphicsZone::key(quint16 num, quint8 forme, bool shiny, bool back, quint8 gender, bool sub) const
{
    return sub ? ((1 << 31) + (back << 30)) : (num + (gender << 16) + (back << 18) + (shiny<<19) + (forme << 20));
}

bool BaseGraphicsZone::event(QEvent * event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

        int spot  = !(helpEvent->pos().x() < width() / 2);
        QToolTip::setFont(QFont("Courier New",8));
        QToolTip::showText(helpEvent->globalPos(), tooltips[spot]);
    }
    return QGraphicsView::event(event);
}

