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
private slots:
    void p_toggled(bool);
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
};

class TB_PokemonItem : public QGraphicsPixmapItem
{
public:
    TB_PokemonItem(PokeTeam *item);
    ~TB_PokemonItem();

    PokeTeam *poke;
};

class PokemonBox : public QGraphicsView
{
    Q_OBJECT
public:
    PokemonBox(int num);

    void addPokemon(const PokeTeam &poke) throw (QString);
    bool isFull() const;
    int freeSpot() const;
private:
    QVector<TB_PokemonItem*> pokemons;
    int num;
};

class TB_PokemonBoxes : public QWidget
{
    Q_OBJECT
public:
    TB_PokemonBoxes(TeamBuilder *parent);

    void updateBox();
public slots:
    void changePokemon(int newpoke);
private:
    TB_PokemonDetail *m_details;
    TB_PokemonButtons *m_buttons;
    Team *m_team;
    QTabWidget *m_boxes;
};

#endif // BOX_H
