#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"

void MoveGen::init(int gen, int pokenum)
{
    this->gen = gen;
    this->pokenum = pokenum;

    moves[LevelMoves] = PokemonInfo::LevelMoves(pokenum,gen);
    moves[SpecialMoves] = PokemonInfo::SpecialMoves(pokenum,gen);
    moves[EggMoves] = PokemonInfo::EggMoves(pokenum,gen);
    moves[TutorMoves] = PokemonInfo::TutorMoves(pokenum,gen);
    moves[TMMoves] = PokemonInfo::TMMoves(pokenum, gen);
}

void MovesPerPoke::init(int poke)
{
    this->pokenum = poke;

    gens[0].init(3,poke);
    gens[1].init(4,poke);
}

void PokeMovesDb::init()
{
    for (int i =0; i < PokemonInfo::NumberOfPokemons(); i++) {
        MovesPerPoke p;
        p.init(i);
        pokes.push_back(p);
    }
}

void PokeMovesDb::save()
{
    QFile files[2][5];

    for (int gen = 3; gen <= 4; gen++) {
        QString genS = "db/pokes/" + QString::number(gen) + "G_";
        files[gen-3][LevelMoves].setFileName(genS + "level_moves.txt");
        files[gen-3][EggMoves].setFileName(genS + "egg_moves.txt");
        files[gen-3][TutorMoves].setFileName(genS + "tutor_moves.txt");
        files[gen-3][SpecialMoves].setFileName(genS + "special_moves.txt");
        files[gen-3][TMMoves].setFileName(genS + "tm_and_hm_moves.txt");
    }
    for (int gen = 3; gen <= 4; gen++) {
        for (int i = 0; i < 5; i++) {
            files[gen-3][i].open(QIODevice::WriteOnly);

            for (int p = 0; p < PokemonInfo::NumberOfPokemons(); p++) {
                QString s;
                bool start = true;

                foreach(int move, pokes[p].gens[gen-3].moves[i]) {
                    if (!start) {
                        s += " ";
                    } else {
                        start = false;
                    }
                    s += QString::number(move);
                }

                files[gen-3][i].write((s+"\n").toUtf8());
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
    ui->gen4->setChecked(true);
    ui->pokeMoves->setCurrentIndex(0);

    currentPoke = 0;

    PokemonInfo::init("db/pokes/");
    MoveInfo::init("db/moves/");
    database.init();

    connect(ui->save, SIGNAL(triggered()), SLOT(save()));
    connect(ui->gen4, SIGNAL(toggled(bool)), SLOT(setPokeByNick()));

    /**********************
         Pokemons
    **********************/
    ui->pokemonList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->pokemonList->setSelectionMode(QAbstractItemView::SingleSelection);

    /* Adding the poke names */
    for (int i = 0; i < PokemonInfo::NumberOfPokemons(); i++)
    {
        QIdListWidgetItem *it= new QIdListWidgetItem(i, PokemonInfo::Name(i));
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

void MainWindow::switchToPokemon(int num)
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
    return ui->gen3->isChecked() ? 3 : 4;
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
