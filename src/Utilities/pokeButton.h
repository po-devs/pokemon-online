#ifndef POKEBUTTON_H
#define POKEBUTTON_H

#include <QPushButton>
#include <QPoint>
#include <QPair>

class pokeButton : public QPushButton
{
    Q_OBJECT

public:
    pokeButton(QWidget * parent =0);
    pokeButton(QString text=QString(""),QWidget * parent =0);
    pokeButton(QIcon icon = QIcon(""), QString text=QString(""),QWidget * parent =0);
    ~pokeButton();

    int index;
signals:
    void changePokemonOrder(QPair<int /*pokemon1*/,int /*pokemon2*/>echange);
    void changePokemonBase(int indexBody,int pokenum);

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dragMoveEvent(QDragMoveEvent * event);
    void dropEvent(QDropEvent * event);

private:
    void startDrag();

    QPoint startPos;
};

#endif // POKEBUTTON_H
