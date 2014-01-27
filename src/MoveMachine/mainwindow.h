#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <PokemonInfo/geninfo.h>
#include <QMainWindow>
#include <QListWidgetItem>
#include <PokemonInfo/pokemonstructs.h>

namespace Ui {
    class MainWindow;
}

enum {
    LevelMoves,
    TutorMoves,
    EggMoves,
    SpecialMoves,
    TMMoves,
    PreEvoMoves,
    DreamWorldMoves,
    AllMoves
};

struct MoveGen
{
    Pokemon::gen gen;
    Pokemon::uniqueId id;
    QSet<int> moves[8];

    void init(Pokemon::gen gen, Pokemon::uniqueId id);
};

struct MovesPerPoke
{
    Pokemon::uniqueId id;
    QHash<Pokemon::gen, MoveGen> gens;

    void init(Pokemon::uniqueId id);
};

struct PokeMovesDb
{
    void init();

    QHash<Pokemon::uniqueId, MovesPerPoke> pokes;

    void save();
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void pokemonChosen(QListWidgetItem*);
    void setPokeByNick();
    void save();
    void moveChosen(QListWidgetItem*);
    void moveDeleted(QListWidgetItem*);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    Pokemon::uniqueId currentPoke;

    void switchToPokemon(Pokemon::uniqueId);
    Pokemon::gen gen();
    PokeMovesDb database;
    void addMoves(Pokemon::gen gen, int cat, QListWidget *container);
};

#endif // MAINWINDOW_H
