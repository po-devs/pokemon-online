namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"

void MoveGen::init(int gen, Pokemon::uniqueId pokenum)
{
    this->gen = gen;
    this->id = pokenum;

    moves[LevelMoves] = PokemonInfo::LevelMoves(pokenum,gen);
    moves[SpecialMoves] = PokemonInfo::SpecialMoves(pokenum,gen);
    moves[EggMoves] = PokemonInfo::EggMoves(pokenum,gen);
    moves[TutorMoves] = PokemonInfo::TutorMoves(pokenum,gen);
    moves[TMMoves] = PokemonInfo::TMMoves(pokenum, gen);
}

void MovesPerPoke::init(Pokemon::uniqueId poke)
{
    this->id = poke;

    gens[0].init(3,poke);
    gens[1].init(4,poke);
    gens[2].init(5,poke);
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
//    for (int i =0; i < PokemonInfo::NumberOfPokemons(); i++) {
//        int preEvo = PokemonInfo::PreEvo(i);

//        if (preEvo != 0) {
//            pokes[i].gens[0].moves[LevelMoves].unite(pokes[preEvo].gens[0].moves[LevelMoves]);
//        }
//    }
}

void PokeMovesDb::save()
{
    QFile files[3][5];

    for (int gen = 3; gen <= 5; gen++) {
        QString genS = "db/pokes/" + QString::number(gen) + "G_";
        files[gen-3][LevelMoves].setFileName(genS + "level_moves.txt");
        files[gen-3][EggMoves].setFileName(genS + "egg_moves.txt");
        files[gen-3][TutorMoves].setFileName(genS + "tutor_moves.txt");
        files[gen-3][SpecialMoves].setFileName(genS + "special_moves.txt");
        files[gen-3][TMMoves].setFileName(genS + "tm_and_hm_moves.txt");
    }

    QList<Pokemon::uniqueId> ids = PokemonInfo::AllIds();
    qSort(ids);
    for (int gen = 3; gen <= 5; gen++) {
        for (int i = 0; i < 5; i++) {
            files[gen-3][i].open(QIODevice::WriteOnly);
            foreach (Pokemon::uniqueId id, ids) {
                if (PokemonInfo::IsAesthetic(id) || pokes[id].gens[gen-3].moves[i].size() == 0)
                    continue;

                QList<int> moves = pokes[id].gens[gen-3].moves[i].toList();
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

                files[gen-3][i].write(id.toLine(s+"\n").toUtf8());
            }
            files[gen-3][i].close();
        }
    }
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

    QFile in("db/pokes/tmmoves.txt");
    in.open(QIODevice::ReadOnly);
    QString s = QString::fromUtf8(in.readAll());
    in.close();

    QStringList pokes  = s.split('\n');
    QStringList out;

    foreach ( QString poke, pokes ) {
        QString ids = poke.section(' ', 0, 0);
        Pokemon::uniqueId id(ids.section(':', 0, 0).toInt(), ids.section(':', 1, 1).toInt());

        QString data = poke.section(' ', 1);
        QStringList moves = data.split(", ");

        QStringList moveNums;

        foreach( QString move, moves ) {
            if (move.length() == 0)
                continue;

            int num = MoveInfo::Number(move);

            if (num == 0) {
                qDebug() << move;
                exit(1);
            }
            moveNums.push_back(QString::number(num));
        }

        moveNums.sort();

        QString body = moveNums.join(" ");
        out.push_back(id.toLine(body));
    }

    QFile outfile("db/pokes/5G_tm_and_hm_moves.txt");
    outfile.open(QIODevice::WriteOnly);
    outfile.write(out.join("\n").toUtf8());
    outfile.close();
    exit(0);

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

    connect(ui->save, SIGNAL(triggered()), SLOT(save()));
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
}

void MainWindow::addMoves(int gen, int cat, QListWidget *container)
{
    container->clear();

    foreach(int move, database.pokes[currentPoke].gens[gen-3].moves[cat]) {
        container->addItem(new QIdListWidgetItem(move, MoveInfo::Name(move)));
    }

    container->sortItems();
}

int MainWindow::gen() {
    return ui->gen3->isChecked() ? 3 : (ui->gen4->isChecked() ? 4 : 5);
}

void MainWindow::moveChosen(QListWidgetItem *it)
{
    int movenum = ((QIdListWidgetItem *)it)->id();
    int cat = ui->pokeMoves->currentIndex();

    QSet<int> &moves = database.pokes[currentPoke].gens[gen()-3].moves[cat];

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
    }

    QSet<int> &moves = database.pokes[currentPoke].gens[gen()-3].moves[cat];

    moves.remove(((QIdListWidgetItem*)it)->id());
    addMoves(gen(), cat, container);
}
