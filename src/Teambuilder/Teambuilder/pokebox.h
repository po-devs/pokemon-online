#ifndef POKEBOX_H
#define POKEBOX_H

#include <QGraphicsView>
#include <QPixmap>

class PokeTeam;
class PokeBoxItem;

class PokeBox : public QGraphicsView
{
    Q_OBJECT

public:
    PokeBox(int boxNum, const QString &file);

    void deleteBox();
    void saveBox();
    void loadBox();
    void addPokemonToBox(const PokeTeam &poke, int slot = -1);

    PokeTeam *getCurrent();
    void deleteCurrent();
    void changeCurrent(const PokeTeam &poke);
    void changeCurrentSpot(int newSpot);

    QString getBoxName() const;
    void setName(const QString &name);
    void reName(const QString &newName);
    bool isFull() const;
    bool isEmpty() const;
    int freeSpot() const;

    int currentSlot() const;
    int getNum() const;
    void setNum(int number);

    int getNumOf(const PokeBoxItem *item) const;

    static QString getBoxPath();
public slots:
    void loadIfNeeded();
signals:
    void switchWithTeam(int boxNumber, int boxSlot, int teamSlot);
    void show(PokeTeam *team);

protected:
    void addGraphicsItem(int spot);
    QPointF calculatePos(int spot, const QSize& itemSize = QSize(32,32));
    int calculateSpot(const QPoint &graphicViewPos);
    void drawBackground(QPainter *painter, const QRectF &rect);
    PokeBoxItem* currentItem();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

    void updateSpots(int i, int j);
private:
    QVector<PokeBoxItem*> m_Pokemons;
    QString m_Name;
    int m_Num;
    int currentPokemon;

    bool isLoaded;

    QPixmap m_Empty, m_Occupied, m_Hover, m_Clicked;
    QPoint oldMousePos;
};

#endif // POKEBOX_H
