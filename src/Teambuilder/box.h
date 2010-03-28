#ifndef BOX_H
#define BOX_H
#include <QtGui>

class PokeTeam;
class AvatarBox;
class Team;
class TeamBuilder;

class TB_PokemonDetail : public QFrame
{
    Q_OBJECT
public:
    TB_PokemonDetail();

    void changePoke(PokeTeam *poke, int num);
    void updatePoke();
private:
    PokeTeam *poke;
    int num;

    QLabel *m_name, *m_nick, *m_num, *m_gender, *m_level, *m_type1, *m_type2, *m_nature, *m_item;
    QLabel *m_moves[4];

    AvatarBox *m_pic;
};

class PokemonBoxButton : public QPushButton
{
    Q_OBJECT
public:
    PokemonBoxButton(int num);
public:
    static QPixmap *theicon;
    static QPixmap *theglowedicon;
};

class TB_PokemonButtons : public QFrame
{
    Q_OBJECT
public:
    TB_PokemonButtons();
signals:
    void buttonChecked(int button);

public:
    PokemonBoxButton *buttons[6];
};

class TB_PokemonItem : public QGraphicsPixmapItem
{
public:
    TB_PokemonItem(PokeTeam *item);
    ~TB_PokemonItem();

    void changePoke(PokeTeam *poke);

    PokeTeam *poke;
};

class PokemonBox : public QGraphicsView
{
    Q_OBJECT
public:
    PokemonBox(int num);

    void addPokemon(const PokeTeam &poke) throw (QString);

    /* Gets the team held by the current item */
    PokeTeam *getCurrent() throw (QString);
    /* Deletes the current item  */
    void deleteCurrent() throw (QString);

    void changeCurrent(const PokeTeam &poke) throw (QString);

    bool isFull() const;
    bool isEmpty() const;
    int freeSpot() const;
    /* Sets currentPoke as the first spot available */
    void updateCurrentPoke();

protected:
    void addGraphicsItem(int spot);
    QPointF calculatePos(int spot, const QSize& itemSize = QSize(32,32));
    /*
     * Gets the spot corresponding to that pos.
     * Returns -1 on failure
    */
    int calculateSpot(const QPoint &graphViewPos);

    void drawBackground(QPainter *painter, const QRectF &rect);
    void changeCurrentSpot(int newspot);
    TB_PokemonItem* currentItem();

    void mousePressEvent(QMouseEvent *event);

    static QPixmap *selBg;
private:
    QVector<TB_PokemonItem*> pokemons;
    int num;
    int currentPoke;
};

class TB_PokemonBoxes : public QWidget
{
    Q_OBJECT
public:
    TB_PokemonBoxes(TeamBuilder *parent);

    void updateBox();
    void updateSpot(int i);
public slots:
    void changeCurrentTeamPokemon(int newpoke);
    void store();
    void withdraw();
    void switchP();
    void deleteP();
protected:
    PokeTeam *currentPokeTeam();
private:
    TB_PokemonDetail *m_details;
    TB_PokemonButtons *m_buttons;
    Team *m_team;
    int currentPoke;
    QTabWidget *m_boxes;
    PokemonBox * boxes[6];

    PokemonBox *currentBox() {
        return boxes[m_boxes->currentIndex()];
    }
};

#endif // BOX_H
