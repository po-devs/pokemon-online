#ifndef TEAMBODY_H
#define TEAMBODY_H

#include <QWidget>

#include <QPushButton>
#include "../../PokemonInfo/pokemoninfo.h"

class TB_PokemonBody;
class TeamHolder;
class QStackedWidget;
class DockAdvanced;
class QSplitter;
class QStringListModel;
class QLabel;
class TeamPokeButton;
class QAbstractItemModel;

class TB_TeamBody: public QWidget
{
    Q_OBJECT
public:
    TB_TeamBody(QWidget *parent, TeamHolder *team, int gen, QAbstractItemModel *pokeModel);
    ~TB_TeamBody();

    void updateTeam();
    void updatePoke(int num);
    void changeGeneration(int gen);
    bool isAdvancedOpen() const;
    void reloadItems(bool showAll);
private slots:
    void changeIndex();
    void updateButton();
    void changePokemonOrder(QPair<int,int>);
    void changePokemonBase(int slot, const Pokemon::uniqueId &num);
    void advancedClicked(bool separateWindow);
    void advancedClicked(int index, bool separateWindow);
    void advancedDestroyed();
    void indexNumChanged(Pokemon::uniqueId pokeNum);
public:
    TB_PokemonBody *pokeBody[6];
private:
    TeamPokeButton *pokeButtons[6];
    QStackedWidget *body;

    QSplitter *splitter;
    bool advSepWindow;
    DockAdvanced * m_dockAdvanced;
    DockAdvanced * dockAdvanced() const {
        return m_dockAdvanced;
    }
    void createDockAdvanced(bool sepWindow);

    TeamHolder *m_team;
    TeamHolder* trainerTeam() {
        return m_team;
    }

    int gen;

    QStringListModel *itemsModel, *natureModel;
    QAbstractItemModel *pokeModel;
    QWidget *upParent;

    void saveAdvancedState();
    void restoreAdvancedState();

    friend class DockAdvanced;
};

class TeamPokeButton : public QPushButton
{
    Q_OBJECT
public:
    TeamPokeButton(int num, Pokemon::uniqueId poke=Pokemon::NoPoke, int level=100, int item = 0);

    void changeInfos(Pokemon::uniqueId poke=Pokemon::NoPoke, int level=100, int item = 0);
    int num() const {return m_num;}
signals:
    void changePokemonOrder(QPair<int,int> exchange);
    void changePokemonBase(int num, Pokemon::uniqueId);

protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void startDrag();
private:
    QLabel *pokeIcon, *level, *itemIcon;
    int m_num;

    QPoint startPos;
};

#endif // TEAMBODY_H
