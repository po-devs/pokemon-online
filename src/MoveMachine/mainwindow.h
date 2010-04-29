#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

namespace Ui {
    class MainWindow;
}

enum {
    LevelMoves,
    TutorMoves,
    EggMoves,
    SpecialMoves,
    TMMoves
};

struct MoveGen
{
    int gen;
    int pokenum;
    QSet<int> moves[5];

    void init(int gen, int pokenum);
};

struct MovesPerPoke
{
    int pokenum;
    MoveGen gens[2];

    void init(int poke);
};

struct PokeMovesDb
{
    void init();

    QList<MovesPerPoke> pokes;

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
    int currentPoke;

    void switchToPokemon(int);
    int gen();
    PokeMovesDb database;
    void addMoves(int gen, int cat, QListWidget *container);
};

#endif // MAINWINDOW_H
