#ifndef POKEBUTTON_H
#define POKEBUTTON_H

#include <QPushButton>
#include <QElapsedTimer>

namespace Ui {
    class PokeButton;
}

class PokeTeam;

class PokeButton : public QPushButton
{
    Q_OBJECT

public:
    explicit PokeButton(QWidget *parent = 0);
    ~PokeButton();

    void setNumber(int x);
    void setPokemon(PokeTeam &poke);
    void updateAll();
signals:
    void pokemonOrderChanged(int start, int end);
    void doubleClicked();
    void dropEventReceived(int index, QDropEvent *e);
protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void keyReleaseEvent(QKeyEvent *);
    void startDrag();
private:
    Ui::PokeButton *ui;
    QPoint startPos;
    QElapsedTimer lastPress;

    PokeTeam *m_poke;
    int num;
    PokeTeam &poke() {return *m_poke;}
};

#endif // POKEBUTTON_H
