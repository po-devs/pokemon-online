namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include <QCompleter>
#include <QMessageBox>

void MoveGen::init(int gen, Pokemon::uniqueId pokenum)
{
    this->gen = gen;
    this->id = pokenum;

    moves[LevelMoves] = PokemonInfo::LevelMoves(pokenum,gen);
    moves[SpecialMoves] = PokemonInfo::SpecialMoves(pokenum,gen);
    moves[EggMoves] = PokemonInfo::EggMoves(pokenum,gen);
    moves[TutorMoves] = PokemonInfo::TutorMoves(pokenum,gen);
    moves[TMMoves] = PokemonInfo::TMMoves(pokenum, gen);
    moves[PreEvoMoves] = PokemonInfo::PreEvoMoves(pokenum, gen);
    if (gen == 5)
        moves[DreamWorldMoves] = PokemonInfo::dreamWorldMoves(pokenum);
}

void MovesPerPoke::init(Pokemon::uniqueId poke)
{
    this->id = poke;

    gens[0].init(1,poke);
    gens[1].init(2,poke);
    gens[2].init(3,poke);
    gens[3].init(4,poke);
    gens[4].init(5,poke);
}

void PokeMovesDb::init()
{
    foreach(Pokemon::uniqueId id, PokemonInfo::AllIds()) {
        if (PokemonInfo::IsAesthetic(id))
            continue;

        MovesPerPoke p;
        p.init(id);
        pokes[id] = p;
    }

    /* Code to give evos the moves of their pre evos */
    for (int i =0; i < PokemonInfo::TrueCount(5); i++) {
        int preEvo = PokemonInfo::PreEvo(i);

        if (preEvo != 0) {
            pokes[i].gens[0].moves[PreEvoMoves] = pokes[preEvo].gens[0].moves[LevelMoves];
            pokes[i].gens[0].moves[PreEvoMoves].unite(pokes[preEvo].gens[0].moves[PreEvoMoves]);
            pokes[i].gens[0].moves[PreEvoMoves].unite(pokes[preEvo].gens[0].moves[SpecialMoves]);
            pokes[i].gens[0].moves[PreEvoMoves].unite(pokes[preEvo].gens[0].moves[TMMoves]);
            pokes[i].gens[0].moves[PreEvoMoves].unite(pokes[preEvo].gens[0].moves[TutorMoves]);
            pokes[i].gens[0].moves[PreEvoMoves].subtract(pokes[i].gens[0].moves[LevelMoves]);
            pokes[i].gens[0].moves[PreEvoMoves].subtract(pokes[i].gens[0].moves[TMMoves]);
            pokes[i].gens[1].moves[PreEvoMoves] = pokes[preEvo].gens[1].moves[LevelMoves];
            pokes[i].gens[1].moves[PreEvoMoves].unite(pokes[preEvo].gens[1].moves[SpecialMoves]);
            pokes[i].gens[1].moves[PreEvoMoves].unite(pokes[preEvo].gens[1].moves[PreEvoMoves]);
            pokes[i].gens[1].moves[PreEvoMoves].unite(pokes[preEvo].gens[1].moves[TMMoves]);
            pokes[i].gens[1].moves[PreEvoMoves].unite(pokes[preEvo].gens[1].moves[TutorMoves]);
            pokes[i].gens[1].moves[PreEvoMoves].subtract(pokes[i].gens[1].moves[LevelMoves]);
            pokes[i].gens[1].moves[PreEvoMoves].subtract(pokes[i].gens[1].moves[TutorMoves]);
            pokes[i].gens[1].moves[PreEvoMoves].subtract(pokes[i].gens[1].moves[TMMoves]);
            pokes[i].gens[1].moves[EggMoves].unite(pokes[preEvo].gens[1].moves[EggMoves]);
            pokes[i].gens[2].moves[PreEvoMoves] = pokes[preEvo].gens[2].moves[LevelMoves];
            pokes[i].gens[2].moves[PreEvoMoves].unite(pokes[preEvo].gens[2].moves[SpecialMoves]);
            pokes[i].gens[2].moves[PreEvoMoves].unite(pokes[preEvo].gens[2].moves[PreEvoMoves]);
            pokes[i].gens[2].moves[PreEvoMoves].unite(pokes[preEvo].gens[2].moves[TMMoves]);
            pokes[i].gens[2].moves[PreEvoMoves].unite(pokes[preEvo].gens[2].moves[TutorMoves]);
            pokes[i].gens[2].moves[PreEvoMoves].subtract(pokes[i].gens[2].moves[LevelMoves]);
            pokes[i].gens[2].moves[PreEvoMoves].subtract(pokes[i].gens[2].moves[TutorMoves]);
            pokes[i].gens[2].moves[PreEvoMoves].subtract(pokes[i].gens[2].moves[TMMoves]);
            pokes[i].gens[3].moves[PreEvoMoves] = pokes[preEvo].gens[3].moves[LevelMoves];
            pokes[i].gens[3].moves[PreEvoMoves].unite(pokes[preEvo].gens[3].moves[SpecialMoves]);
            pokes[i].gens[3].moves[PreEvoMoves].unite(pokes[preEvo].gens[3].moves[PreEvoMoves]);
            pokes[i].gens[3].moves[PreEvoMoves].unite(pokes[preEvo].gens[3].moves[TMMoves]);
            pokes[i].gens[3].moves[PreEvoMoves].unite(pokes[preEvo].gens[3].moves[TutorMoves]);
            pokes[i].gens[3].moves[PreEvoMoves].subtract(pokes[i].gens[3].moves[LevelMoves]);
            pokes[i].gens[3].moves[PreEvoMoves].subtract(pokes[i].gens[3].moves[TutorMoves]);
            pokes[i].gens[3].moves[PreEvoMoves].subtract(pokes[i].gens[3].moves[TMMoves]);
            pokes[i].gens[4].moves[PreEvoMoves] = pokes[preEvo].gens[4].moves[LevelMoves];
            pokes[i].gens[4].moves[PreEvoMoves].unite(pokes[preEvo].gens[4].moves[SpecialMoves]);
            pokes[i].gens[4].moves[PreEvoMoves].unite(pokes[preEvo].gens[4].moves[PreEvoMoves]);
            pokes[i].gens[4].moves[PreEvoMoves].unite(pokes[preEvo].gens[4].moves[TutorMoves]);
            pokes[i].gens[4].moves[PreEvoMoves].subtract(pokes[i].gens[4].moves[LevelMoves]);
            pokes[i].gens[4].moves[PreEvoMoves].subtract(pokes[i].gens[4].moves[TutorMoves]);
            pokes[i].gens[4].moves[PreEvoMoves].subtract(pokes[i].gens[4].moves[TMMoves]);
            //pokes[i].gens[2].moves[EggMoves].unite(pokes[preEvo].gens[2].moves[EggMoves]);
        }
    }
}

void PokeMovesDb::save()
{
    QFile files[5][7];

    for (int gen = 1; gen <= 5; gen++) {
        QString genS = "db/pokes/" + QString::number(gen) + "G_";
        files[gen-1][LevelMoves].setFileName(genS + "level_moves.txt");
        files[gen-1][EggMoves].setFileName(genS + "egg_moves.txt");
        files[gen-1][TutorMoves].setFileName(genS + "tutor_moves.txt");
        files[gen-1][SpecialMoves].setFileName(genS + "special_moves.txt");
        files[gen-1][TMMoves].setFileName(genS + "tm_and_hm_moves.txt");
        files[gen-1][PreEvoMoves].setFileName(genS + "pre_evo_moves.txt");
        files[gen-1][DreamWorldMoves].setFileName(genS + "dw_moves.txt");
    }

    QList<Pokemon::uniqueId> ids = PokemonInfo::AllIds();
    qSort(ids);
    for (int gen = 1; gen <= 5; gen++) {
        for (int i = 0; i < (gen == 5 ? 7 : 6); i++) {
            files[gen-1][i].open(QIODevice::WriteOnly);
            foreach (Pokemon::uniqueId id, ids) {
                if (PokemonInfo::IsAesthetic(id) || pokes[id].gens[gen-1].moves[i].size() == 0)
                    continue;

                QList<int> moves = pokes[id].gens[gen-1].moves[i].toList();
                qSort(moves);

                QString s;
                bool start = true;

                foreach(int move, moves) {
                    if (!start) {
                        s += " ";
                    } else {
                        start = false;
                    }
                    s += QString::number(move);
                }

                files[gen-1][i].write(id.toLine(s+"\n").toUtf8());
            }
            files[gen-1][i].close();
        }
    }
}

QString getLine(const QString & filename, int linenum)
{
    QFile file(filename);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream filestream(&file);

    /* discarding all the uninteresting lines, should find a more effective way */
    for (int i = 0; i < linenum; i++)
    {
        filestream.readLine();
    }

    return filestream.readLine();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->save->setShortcut(Qt::CTRL+Qt::Key_S);
    ui->gen5->setChecked(true);
    ui->pokeMoves->setCurrentIndex(0);

    currentPoke = 0;

    PokemonInfo::init("db/pokes/");
    MoveInfo::init("db/moves/");
    database.init();

//    QFile in("db/pokes/level_to_extract.txt");
//    in.open(QIODevice::ReadOnly);
//    QString s = QString::fromUtf8(in.readAll());
//    in.close();

//    QStringList pokes  = s.split('\n');
//    QStringList out;

//    foreach ( QString poke, pokes ) {
//        Pokemon::uniqueId id(poke.section(':', 0, 0).toInt(), 0);

//        QString data = poke.section(": ", 1);
//        QStringList moves = data.split(", ");

//        QStringList moveNums;

//        foreach( QString move, moves ) {
//            QString name = move.section(": ", 1);
//            moveNums.push_back(QString::number(MoveInfo::Number(name)));
//        }

//        moveNums.sort();

//        QString body = moveNums.join(" ");
//        out.push_back(id.toLine(body));
//    }

//    QFile outfile("db/pokes/5G_level_moves.txt");
//    outfile.open(QIODevice::WriteOnly);
//    outfile.write(out.join("\n").toUtf8());
//    outfile.close();
//    exit(0);

//    QFile in("db/pokes/tmmoves.txt");
//    in.open(QIODevice::ReadOnly);
//    QString s = QString::fromUtf8(in.readAll());
//    in.close();

//    QStringList pokes  = s.split('\n');
//    QStringList out;

//    foreach ( QString poke, pokes ) {
//        QString ids = poke.section(' ', 0, 0);
//        Pokemon::uniqueId id(ids.section(':', 0, 0).toInt(), ids.section(':', 1, 1).toInt());

//        QString data = poke.section(' ', 1);
//        QStringList moves = data.split(", ");

//        QStringList moveNums;

//        foreach( QString move, moves ) {
//            if (move.length() == 0)
//                continue;

//            int num = MoveInfo::Number(move);

//            if (num == 0) {
//                qDebug() << move;
//                exit(1);
//            }
//            moveNums.push_back(QString::number(num));
//        }

//        moveNums.sort();

//        QString body = moveNums.join(" ");
//        out.push_back(id.toLine(body));
//    }

//    QFile outfile("db/pokes/5G_tm_and_hm_moves.txt");
//    outfile.open(QIODevice::WriteOnly);
//    outfile.write(out.join("\n").toUtf8());
//    outfile.close();
//    exit(0);

//    AbilityInfo::init();
//    QFile in("db/abilities/ability_extract.txt");
//    in.open(QIODevice::ReadOnly);
//    QString s = QString::fromUtf8(in.readAll());
//    in.close();

//    QStringList pokes  = s.split('\n');

//    QStringList out[3];

//    for (int i = 0; i < pokes.size(); i++) {
//        QString poke = pokes[i];
//        Pokemon::uniqueId id(i+1, 0);

//        QStringList data = poke.split('/');

//        int abilities[3] = {0,0,0};

//        for (int j = 0; j < data.size(); j++) {
//            QString ab = data[j];

//            if (ab.contains(" (Dream World)"))
//            {
//                ab.replace(" (Dream World)", "");
//                abilities[2] = AbilityInfo::Number(ab);
//            } else {
//                abilities[j] = AbilityInfo::Number(ab);
//            }
//        }

//        for (int j = 0; j < 3; j++) {
//            out[j].push_back(id.toLine(QString::number(abilities[j])));
//        }
//    }

//    for (int i = 0; i < 3; i++) {
//        QFile outfile(QString("db/pokes/poke_ability%1_5G.txt").arg(i+1));
//        outfile.open(QIODevice::WriteOnly);
//        outfile.write(out[i].join("\n").toUtf8());
//        outfile.close();
//    }

//    exit(0);

//    QFile in("db/pokes/egg_moves_to_extract.txt");
//    in.open(QIODevice::ReadOnly);
//    QString s = QString::fromUtf8(in.readAll());
//    in.close();

//    QStringList pokes  = s.split("\n-----\n");
//    QStringList out;

//    foreach ( QString poke, pokes ) {
//        QStringList dataX = poke.split('\n');
//        Pokemon::uniqueId id = PokemonInfo::Number(dataX[0]);

//        qDebug() << dataX[0];
//        if (id == Pokemon::NoPoke) {
//            exit(1);
//        }

//        QStringList moves = dataX;
//        moves.removeAt(0);

//        QStringList moveNums;

//        foreach( QString move, moves ) {
//            qDebug() << move;
//            int num = MoveInfo::Number(move);
//            if (num == 0) {
//                exit(2);
//            }
//            moveNums.push_back(QString::number(num));
//        }

//        moveNums.sort();

//        QString body = moveNums.join(" ");
//        out.push_back(id.toLine(body));
//    }

//    qDebug() << "Success";
//    QFile outfile("db/pokes/5G_egg_moves.txt");
//    outfile.open(QIODevice::WriteOnly);
//    outfile.write(out.join("\n").toUtf8());
//    outfile.close();
//    exit(0);

//    TypeInfo::init();
//    CategoryInfo::init();
//    QFile out("importMoves.txt");
//    out.open(QIODevice::WriteOnly);
//    out.write("Title, Free Text\n");
//    for (int i = 1; i <MoveInfo::NumberOfMoves(); i++) {
//        qDebug() << "move " << i;
//        out.putChar('"');
//        out.write("Data:");
//        out.write(MoveInfo::Name(i).toUtf8());
//        out.putChar('"');
//        out.write(", ");
//        out.putChar('"');
//        QString s =QString(
//"{{{{{format}}}|\n"
//"type=%1|pow=%2|acc=%3|pp=%4|class=%5|description=%6|gen=%7|target=%8|category=%9|flags=%10|kingrock=%11|flinch=%12|recoil=%13|healing=%14|"
//"critrate=%15|minturns=%16|maxturns=%17|\n"
//"{{{1|}}}}}"
//).arg(TypeInfo::Name(MoveInfo::Type(i,5)).toLower(), MoveInfo::PowerS(i,5), MoveInfo::AccS(i,5), QString::number(MoveInfo::PP(i,5))
//      , CategoryInfo::Name(MoveInfo::Category(i, 5)).toLower(), MoveInfo::DetailedDescription(i));
//        for (int g = 1; g <=5; g++) {
//            if (MoveInfo::Exists(i, g)) {
//                s = s.arg(g);
//                break;
//            }
//        }
//        QString categories [] = {
//            "Standard",
//            "Status",
//            "Stat Affecting",
//            "Healing",
//            "Damaging Status",
//            "Stat Modifying and Status",
//            "Damaging Stat Affecting",
//            "Damaging and Self-Stat Modifying",
//            "Draining",
//            "OHKO",
//            "Field",
//            "Zone",
//            "Phazing",
//            "Special"
//        };
//        QString targets [] = {
//            "Chosen",
//            "Partner or user",
//            "Partner",
//            "Me First",
//            "All others",
//            "Opponents",
//            "Party",
//            "User",
//            "All",
//            "Random",
//            "Field",
//            "Opponent's Field",
//            "Own Field",
//            "Indeterminate"
//        };
//        s = s.arg(targets[MoveInfo::Target(i, 5)], categories[MoveInfo::Classification(i, 5)], QString::number(MoveInfo::Flags(i, 5)));
//        s = s.arg(MoveInfo::FlinchByKingRock(i) ? "Yes" : "No", QString::number(MoveInfo::FlinchRate(i, 5)),
//                  QString::number(MoveInfo::Recoil(i, 5)), QString::number(MoveInfo::Healing(i, 5)));
//        s = s.arg(QString::number(MoveInfo::CriticalRaise(i, 5)),QString::number(MoveInfo::MinTurns(i, 5)), QString::number(MoveInfo::MaxTurns(i, 5)));
//        out.write(s.toUtf8());
//        out.putChar('"');
//        out.putChar('\n');
//    }
//    out.close();
//    exit(0);

//    PokemonInfo::init("db/pokes/");
//    MoveInfo::init("db/moves/");
//    TypeInfo::init("db/types/");
//    NatureInfo::init("db/natures/");
//    CategoryInfo::init("db/categories/");
//    AbilityInfo::init("db/abilities/");
//    GenderInfo::init("db/genders/");
//    HiddenPowerInfo::init("db/types/");
//    StatInfo::init("db/status/");
//    QFile out("importStuff.txt");
//    out.open(QIODevice::WriteOnly);
//    out.write("Title, Free Text\n");
//    for (int i = 1; i < PokemonInfo::NumberOfPokemons(); i++) {
//        foreach (Pokemon::uniqueId id, PokemonInfo::Formes(i)) {
//            qDebug() << "Pokemon " << PokemonInfo::Name(id);
//            out.putChar('"');
//            out.write("Data:");
//            out.write(PokemonInfo::Name(id).toUtf8());
//            out.putChar('"');
//            out.write(", ");
//            out.putChar('"');
//            PokeGeneral p;
//            p.num() = id;
//            p.gen() = 5;
//            p.load();
//            qDebug() << "1";
//            QString s =QString(
//"<includeonly>{{{{{format}}}|\n"
//"hp=%1|atk=%2|def=%3|satk=%4|sdef=%5|spd=%6|num=%7|type1=%8|type2=%9|ability1=%10|ability2=%11|dwability=%12|height=%13|weight=%14|"
//"gender=%15|baby=false|dreamworld=unreleased|group1=%16|group2=%17|evochain=%18|formes=%19|id=%20\n"
//"{{{1|}}}}}</includeonly>"
//"<noinclude>{{Code}}"
//"</noinclude>"
//).arg(QString::number(p.stats().baseHp()),QString::number(p.stats().baseAttack()),QString::number(p.stats().baseDefense()),
//      QString::number(p.stats().baseSpAttack()),QString::number(p.stats().baseSpDefense()),QString::number(p.stats().baseSpeed()),
//      QString::number(i),TypeInfo::Name(p.type1()).toLower(), p.type2()==Type::Curse ? "" : TypeInfo::Name(p.type2()).toLower());
//            qDebug() << "2";
//            s = s.arg(AbilityInfo::Name(p.abilities().ab(0)), p.abilities().ab(1) == Ability::NoAbility ? "" : AbilityInfo::Name(p.abilities().ab(1)),
//                      p.abilities().ab(2) == Ability::NoAbility ? "" : AbilityInfo::Name(p.abilities().ab(2)),
//                      PokemonInfo::Height(id), PokemonInfo::WeightS(id), p.genderAvail() == Pokemon::Male ? "male" :
//                      (p.genderAvail() == Pokemon::Female ? "female" : (p.genderAvail() == Pokemon::Neutral ? "neutral" : "both")));
//            qDebug() << "3";
//            s = s.arg(getLine("db/pokes/poke_egg_group_1.txt",i).section(' ', 1), getLine("db/pokes/poke_egg_group_2.txt",i).section(' ', 1));
//            QStringList evoChain;
//            evoChain.push_back(QString::number(i));
//            if (PokemonInfo::PreEvo(i) != i && PokemonInfo::PreEvo(i) != 0) {
//                int x = PokemonInfo::PreEvo(i);
//                evoChain.prepend(QString::number(x));
//                if (PokemonInfo::PreEvo(x) != x && PokemonInfo::PreEvo(x) != 0) {
//                    int y = PokemonInfo::PreEvo(x);
//                    evoChain.prepend(QString::number(y));
//                }
//            }
//            if (PokemonInfo::HasEvolutions(i)) {
//                int x = PokemonInfo::DirectEvos(i)[0];
//                evoChain.append(QString::number(x));
//                if (PokemonInfo::HasEvolutions(x)) {
//                    int y = PokemonInfo::DirectEvos(x)[0];
//                    evoChain.append(QString::number(y));
//                }
//            }
//            QStringList formes;
//            foreach(Pokemon::uniqueId id2,PokemonInfo::Formes(id)) {
//                formes.append(PokemonInfo::Name(id2));
//            }
//            s = s.arg(evoChain.join("/"), formes.join("/"), id.toString());

//            out.write(s.toUtf8());
//            out.putChar('"');
//            out.putChar('\n');
//        }
//    }
//    out.close();
//    exit(0);

//    PokemonInfo::init("db/pokes/");
//    MoveInfo::init("db/moves/");
//    TypeInfo::init("db/types/");
//    NatureInfo::init("db/natures/");
//    CategoryInfo::init("db/categories/");
//    AbilityInfo::init("db/abilities/");
//    GenderInfo::init("db/genders/");
//    HiddenPowerInfo::init("db/types/");
//    StatInfo::init("db/status/");
//    QFile out("importStuff.txt");
//    out.open(QIODevice::WriteOnly);
//    out.write("Title, Free Text\n");
//    for (int i = 1; i < AbilityInfo::NumberOfAbilities(); i++) {
//        out.putChar('"');
//        out.write("Data:");
//        out.write(AbilityInfo::Name(i).toUtf8());
//        out.putChar('"');
//        out.write(", ");
//        out.putChar('"');
//        QString s =QString(
//"<includeonly>{{{{{format}}}|\n"
//"description=%1|\n"
//"{{{1|}}}}}</includeonly>"
//"<noinclude>{{Code}}"
//"</noinclude>"
//).arg(AbilityInfo::Desc(i));
//        out.write(s.toUtf8());
//        out.putChar('"');
//        out.putChar('\n');
//    }
//    out.close();
//    exit(0);
//    QFile in("importStuff.txt");
//    in.open(QIODevice::ReadOnly);

//    QString lines = QString::fromUtf8(in.readAll());
//    in.close();

//    QRegExp r("evochain=([0-9]+)", Qt::CaseSensitive, QRegExp::Wildcard);
//    r.setPatternSyntax(QRegExp::RegExp2);
//    int pos = 0;

//    while ((pos = r.indexIn(lines, pos)) > -1) {
//        lines.replace(pos, r.matchedLength(), "evochain="+PokemonInfo::Name(r.cap(1).toInt()));

//        pos += r.matchedLength();
//    }

//    in.open(QIODevice::WriteOnly);
//    in.write(lines.toUtf8());
//    in.close();
//    exit(0);

    connect(ui->save, SIGNAL(triggered()), SLOT(save()));
    connect(ui->gen1, SIGNAL(toggled(bool)), SLOT(setPokeByNick()));
    connect(ui->gen2, SIGNAL(toggled(bool)), SLOT(setPokeByNick()));
    connect(ui->gen3, SIGNAL(toggled(bool)), SLOT(setPokeByNick()));
    connect(ui->gen4, SIGNAL(toggled(bool)), SLOT(setPokeByNick()));
    connect(ui->gen5, SIGNAL(toggled(bool)), SLOT(setPokeByNick()));

    /**********************
         Pokemons
    **********************/
    ui->pokemonList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->pokemonList->setSelectionMode(QAbstractItemView::SingleSelection);

    /* Adding the poke names */
    QList<Pokemon::uniqueId> ids = PokemonInfo::AllIds();
    qSort(ids);

    foreach(Pokemon::uniqueId id, ids)
    {
        if (PokemonInfo::IsAesthetic(id))
            continue;
        QIdListWidgetItem *it= new QIdListWidgetItem(id.toPokeRef(), PokemonInfo::Name(id));
        ui->pokemonList->addItem(it);
    }

    connect(ui->pokemonList, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(pokemonChosen(QListWidgetItem*)));


    QCompleter *completer = new QCompleter(ui->pokemonName);
    completer->setModel(ui->pokemonList->model());
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->pokemonName->setCompleter(completer);

    connect(completer, SIGNAL(activated(QString)), this, SLOT(setPokeByNick()));
    connect(ui->pokemonName, SIGNAL(returnPressed()), this, SLOT(setPokeByNick()));

    /**********************
        Moves
    **********************/
    ui->moveList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->moveList->setSelectionMode(QAbstractItemView::SingleSelection);

    /* Adding the move names */
    for (int i = 0; i < MoveInfo::NumberOfMoves(); i++)
    {
        QIdListWidgetItem *it= new QIdListWidgetItem(i, MoveInfo::Name(i));
        ui->moveList->addItem(it);
    }
    ui->moveList->sortItems(Qt::AscendingOrder);

    connect(ui->moveList, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(moveChosen(QListWidgetItem*)));
    connect(ui->levelMoves, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(moveDeleted(QListWidgetItem*)));
    connect(ui->specialMoves, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(moveDeleted(QListWidgetItem*)));
    connect(ui->eggMoves, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(moveDeleted(QListWidgetItem*)));
    connect(ui->tutorMoves, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(moveDeleted(QListWidgetItem*)));
    connect(ui->tmMoves, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(moveDeleted(QListWidgetItem*)));
    connect(ui->dwMoves, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(moveDeleted(QListWidgetItem*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::pokemonChosen(QListWidgetItem *it)
{
    switchToPokemon(((QIdListWidgetItem*)(it))->id());
}

void MainWindow::setPokeByNick()
{
    switchToPokemon(PokemonInfo::Number(ui->pokemonName->text()));
}

void MainWindow::switchToPokemon(Pokemon::uniqueId num)
{
    currentPoke = num;

    ui->pokemonName->setText(PokemonInfo::Name(num));

    int gen = this->gen();

    addMoves(gen, LevelMoves, ui->levelMoves);
    addMoves(gen, EggMoves, ui->eggMoves);
    addMoves(gen, SpecialMoves, ui->specialMoves);
    addMoves(gen, TutorMoves, ui->tutorMoves);
    addMoves(gen, TMMoves, ui->tmMoves);
    addMoves(gen, PreEvoMoves, ui->preMoves);
    addMoves(gen, DreamWorldMoves, ui->dwMoves);
}

void MainWindow::addMoves(int gen, int cat, QListWidget *container)
{
    container->clear();

    foreach(int move, database.pokes[currentPoke].gens[gen-1].moves[cat]) {
        container->addItem(new QIdListWidgetItem(move, MoveInfo::Name(move)));
    }

    container->sortItems();
}

int MainWindow::gen() {
    return ui->gen1->isChecked() ? 1 : (ui->gen2->isChecked() ? 2 : (ui->gen3->isChecked() ? 3 : (ui->gen4->isChecked() ? 4 : 5)));
}

void MainWindow::moveChosen(QListWidgetItem *it)
{
    int movenum = ((QIdListWidgetItem *)it)->id();
    int cat = ui->pokeMoves->currentIndex();

    QSet<int> &moves = database.pokes[currentPoke].gens[gen()-1].moves[cat];

    if (moves.contains(movenum)) {
        return;
    }

    moves.insert(movenum);

    QListWidget *container = (QListWidget*)ui->pokeMoves->currentWidget()->children().front();

    addMoves(gen(), cat, container);
}

void MainWindow::save()
{
    database.save();

    QMessageBox::information(this, "Saved", "Changes saved succesfully");
}

void MainWindow::moveDeleted(QListWidgetItem *it)
{
    QListWidget *container = (QListWidget*)(sender());

    int cat=0;

    if (container == ui->levelMoves) {
        cat = LevelMoves;
    } else if (container == ui->eggMoves) {
        cat = EggMoves;
    } else if (container == ui->tutorMoves) {
        cat = TutorMoves;
    } else if (container == ui->specialMoves) {
        cat = SpecialMoves;
    } else if (container == ui->tmMoves) {
        cat = TMMoves;
    } else if (container == ui->preMoves) {
        cat = PreEvoMoves;
    } else if (container == ui->dwMoves) {
        cat = DreamWorldMoves;
    }

    QSet<int> &moves = database.pokes[currentPoke].gens[gen()-1].moves[cat];

    moves.remove(((QIdListWidgetItem*)it)->id());
    addMoves(gen(), cat, container);
}
