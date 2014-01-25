#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <PokemonInfo/pokemoninfo.h>
#include <Utilities/otherwidgets.h>
#include <QCompleter>
#include <QMessageBox>

QHash<Pokemon::gen, QHash<Pokemon::uniqueId, PokemonMoves> > moves;

void MoveGen::init(Pokemon::gen gen, Pokemon::uniqueId pokenum)
{
    this->gen = gen;
    this->id = pokenum;

    moves[LevelMoves] = ::moves[gen][id].levelMoves;
    moves[SpecialMoves] = ::moves[gen][id].specialMoves;
    moves[EggMoves] = ::moves[gen][id].eggMoves;
    moves[TutorMoves] = ::moves[gen][id].tutorMoves;
    moves[TMMoves] = ::moves[gen][id].TMMoves;
    moves[PreEvoMoves] = ::moves[gen][id].preEvoMoves;
    if (gen.num == 5) {
        moves[DreamWorldMoves] = ::moves[gen][id].dreamWorldMoves;
    }
}

void MovesPerPoke::init(Pokemon::uniqueId poke)
{
    this->id = poke;

    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++) {
        for (int j = 0; j < GenInfo::NumberOfSubgens(i); j++) {
            gens[Pokemon::gen(i,j)].init(Pokemon::gen(i,j),poke);
        }
    }
}

void PokeMovesDb::init()
{
    foreach(Pokemon::gen gen, GenInfo::AllSubGens()) {
        PokemonInfo::Gen data;
        data.load("db/pokes/", gen);
        moves[gen] = data.m_Moves;
    }

    foreach(Pokemon::uniqueId id, PokemonInfo::AllIds()) {
        if (PokemonInfo::IsForme(id) && id.pokenum != Pokemon::Rotom && id.pokenum != Pokemon::Kyurem && id.pokenum != Pokemon::Wormadam && id.pokenum != Pokemon::Meowstic)
            continue;

        MovesPerPoke p;
        p.init(id);
        pokes[id] = p;
    }

    /* Code to give evos the moves of their pre evos */
    for (int _i = 0; _i < PokemonInfo::TrueCount(); _i++) {
        QVector<int> vpokes;
        vpokes.push_back(_i);
        int preEvo = PokemonInfo::PreEvo(_i);

        if (_i == Pokemon::Wormadam) {
            vpokes.push_back(Pokemon::Wormadam_G);
            vpokes.push_back(Pokemon::Wormadam_S);
        }

        if (preEvo == 0) {
            vpokes.clear();
        }

        foreach(int i, vpokes) {
            for (int j = GenInfo::GenMin(); j <= GenInfo::GenMax(); j++) {
                for (int k = 0; k < GenInfo::NumberOfSubgens(j); k++) {
                    Pokemon::gen g(j,k);

                    pokes[i].gens[g].moves[PreEvoMoves] = pokes[preEvo].gens[g].moves[LevelMoves];
                    pokes[i].gens[g].moves[PreEvoMoves].unite(pokes[preEvo].gens[g].moves[SpecialMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].unite(pokes[preEvo].gens[g].moves[PreEvoMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].unite(pokes[preEvo].gens[g].moves[TutorMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].unite(pokes[preEvo].gens[g].moves[TMMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].unite(pokes[preEvo].gens[g].moves[EggMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].unite(pokes[preEvo].gens[g].moves[DreamWorldMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].subtract(pokes[i].gens[g].moves[LevelMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].subtract(pokes[i].gens[g].moves[TutorMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].subtract(pokes[i].gens[g].moves[TMMoves]);
                    pokes[i].gens[g].moves[PreEvoMoves].subtract(pokes[i].gens[g].moves[EggMoves]);
                    //pokes[i].gens[g].moves[PreEvoMoves].subtract(pokes[i].gens[g].moves[DreamWorldMoves]);
                }
            }
        }
    }
}

void PokeMovesDb::save()
{
    QList<Pokemon::uniqueId> ids = PokemonInfo::AllIds();
    qSort(ids);
    for (int gen = GenInfo::GenMin(); gen <= GenInfo::GenMax(); gen++) {
        for (int sub = 0; sub < GenInfo::NumberOfSubgens(gen); sub++) {
            QFile files[7];

            QString genS = "db/pokes/" + QString::number(gen) + "G/Subgen " + QString::number(sub) + "/";
            files[LevelMoves].setFileName(genS + "level_moves.txt");
            files[EggMoves].setFileName(genS + "egg_moves.txt");
            files[TutorMoves].setFileName(genS + "tutor_moves.txt");
            files[SpecialMoves].setFileName(genS + "special_moves.txt");
            files[TMMoves].setFileName(genS + "tm_and_hm_moves.txt");
            files[PreEvoMoves].setFileName(genS + "pre_evo_moves.txt");
            files[DreamWorldMoves].setFileName(genS + "dw_moves.txt");

            Pokemon::gen g(gen, sub);

            for (int i = 0; i < (gen == 5 ? 7 : 6); i++) {
                foreach (Pokemon::uniqueId id, ids) {
                    if ((id.isForme() && id.pokenum != Pokemon::Rotom && id.pokenum != Pokemon::Kyurem && id.pokenum != Pokemon::Wormadam)) {
                        continue;
                    }
                    if (!pokes[id].gens.contains(g) || pokes[id].gens[g].moves[i].count() == 0) {
                        continue;
                    }
                    pokes[id].gens[Pokemon::gen(gen, -1)].moves[i].unite(pokes[id].gens[g].moves[i]);

                    QList<int> moves = pokes[id].gens[g].moves[i].toList();

                    if (moves.size() == 0) {
                        continue;
                    }

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

                    if (!files[i].isOpen()) {
                        files[i].open(QIODevice::WriteOnly);
                    }
                    files[i].write(id.toLine(s+"\n").toUtf8());
                }
                if (files[i].isOpen()) {
                    files[i].close();
                }
            }
        }

        QFile files[8];

        QString genS = "db/pokes/" + QString::number(gen) + "G/";
        files[LevelMoves].setFileName(genS + "level_moves.txt");
        files[EggMoves].setFileName(genS + "egg_moves.txt");
        files[TutorMoves].setFileName(genS + "tutor_moves.txt");
        files[SpecialMoves].setFileName(genS + "special_moves.txt");
        files[TMMoves].setFileName(genS + "tm_and_hm_moves.txt");
        files[PreEvoMoves].setFileName(genS + "pre_evo_moves.txt");
        files[DreamWorldMoves].setFileName(genS + "dw_moves.txt");
        files[AllMoves].setFileName(genS + "all_moves.txt");

        Pokemon::gen g(gen, -1);

        for (int i = 0; i < 8; i++) {
            foreach (Pokemon::uniqueId id, ids) {
                if ((id.isForme() && id.pokenum != Pokemon::Rotom && id.pokenum != Pokemon::Kyurem && id.pokenum != Pokemon::Wormadam)) {
                    continue;
                }
                if (!pokes[id].gens.contains(g) || pokes[id].gens[g].moves[i].count() == 0) {
                    continue;
                }

                pokes[id].gens[g].moves[AllMoves].unite(pokes[id].gens[g].moves[i]);

                QList<int> moves = pokes[id].gens[g].moves[i].toList();

                if (moves.size() == 0) {
                    continue;
                }

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

                if (!files[i].isOpen()) {
                    files[i].open(QIODevice::WriteOnly);
                }
                files[i].write(id.toLine(s+"\n").toUtf8());

                if (i == AllMoves && gen != 2 && gen < GenInfo::GenMax()) {
                    pokes[id].gens[Pokemon::gen(gen+1,-1)].moves[AllMoves].unite(pokes[id].gens[g].moves[AllMoves]);
                }
            }
            if (files[i].isOpen()) {
                files[i].close();
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->save->setShortcut(Qt::CTRL+Qt::Key_S);
    ui->pokeMoves->setCurrentIndex(0);

    currentPoke = 0;

    PokemonInfoConfig::setLastSubgenToWhole(true);

    GenInfo::init("db/gens/");
    PokemonInfo::init("db/pokes/");
    MoveInfo::init("db/moves/");
    database.init();

    connect(ui->save, SIGNAL(triggered()), SLOT(save()));

    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++) {
        for (int j = 0; j < GenInfo::NumberOfSubgens(i); j++) {
            ui->gen->addItem(GenInfo::Version(Pokemon::gen(i, j)), QVariant::fromValue(Pokemon::gen(i,j)));
        }
    }
    ui->gen->setCurrentIndex(ui->gen->count()-1);

    connect(ui->gen, SIGNAL(currentIndexChanged(int)), SLOT(setPokeByNick()));

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
        if (id.isForme() && id.pokenum != Pokemon::Rotom && id.pokenum != Pokemon::Kyurem && id.pokenum != Pokemon::Wormadam && id.pokenum != Pokemon::Meowstic)
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

    Pokemon::gen gen = this->gen();

    addMoves(gen, LevelMoves, ui->levelMoves);
    addMoves(gen, EggMoves, ui->eggMoves);
    addMoves(gen, SpecialMoves, ui->specialMoves);
    addMoves(gen, TutorMoves, ui->tutorMoves);
    addMoves(gen, TMMoves, ui->tmMoves);
    addMoves(gen, PreEvoMoves, ui->preMoves);
    addMoves(gen, DreamWorldMoves, ui->dwMoves);
}

void MainWindow::addMoves(Pokemon::gen gen, int cat, QListWidget *container)
{
    container->clear();

    foreach(int move, database.pokes[currentPoke].gens[gen].moves[cat]) {
        container->addItem(new QIdListWidgetItem(move, MoveInfo::Name(move)));
    }

    container->sortItems();
}

Pokemon::gen MainWindow::gen() {
    return ui->gen->itemData(ui->gen->currentIndex()).value<Pokemon::gen>();
}

void MainWindow::moveChosen(QListWidgetItem *it)
{
    int movenum = ((QIdListWidgetItem *)it)->id();
    int cat = ui->pokeMoves->currentIndex();

    QSet<int> &moves = database.pokes[currentPoke].gens[gen()].moves[cat];

    if (moves.contains(movenum)) {
        return;
    }

    moves.insert(movenum);

    QListWidget *container = ui->pokeMoves->currentWidget()->findChild<QListWidget*>();

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

    QSet<int> &moves = database.pokes[currentPoke].gens[gen()].moves[cat];

    moves.remove(((QIdListWidgetItem*)it)->id());
    addMoves(gen(), cat, container);
}
