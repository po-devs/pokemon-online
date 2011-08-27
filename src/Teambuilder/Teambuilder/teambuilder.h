#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QtGui>
#include <QPair>

#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/otherwidgets.h"
#include "centralwidget.h"

class TB_PokemonBody;
class TB_TrainerBody;
class TB_PokemonBoxes;
class Pokedex;
class TB_TeamBody;
class TB_EVManager;
class DockAdvanced;
class TB_Advanced;

class PokeTableModel;
class PokeMovesModel;
class TrainerTeam;
class Team;
class PokeTeam;
class MainEngine;

/* The Teambuilder!! */
class TeamBuilder : public QLabel, public CentralWidgetInterface
{
    Q_OBJECT
public:
    TeamBuilder(TrainerTeam *team);
    ~TeamBuilder();

    TrainerTeam *trainerTeam();
    Team *team();
    TrainerTeam *trainerTeam() const;
    Team *team() const;

    /* Create a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainEngine *w);

    QSize defaultSize() {
        return  QSize(785,610);
    }

public slots:
    void saveTeam();
    void loadTeam();
    void newTeam();
    void pokeChanged(int poke);
    void clickOnDone();
    void updateAll();
    void importFromTxt();
    void exportToTxt();
    void importDone(const QString &text);
    void showNoFrame();
    void changeItemDisplay(bool allItems);
    void enforceMinLevels(bool enforce);
    void setTierList(const QStringList &tiers);

private:
    enum StackWidgets {
        TrainerW=0,
        TeamW=1,
        BoxesW=2,
        PokedexW=3,
        LastW
    };

private slots:
    void changeMod();
    void setNoMod();
    void changeToTrainer();
    void changeToTeam();
    void changeToBoxes();
    void changeToPokedex();
    void changeZone();
    void genChanged();

signals:
    void done();
private:
    QStackedWidget *m_body;
    TB_TrainerBody *m_trainerBody;
    TB_TeamBody *m_teamBody;
    TB_PokemonBoxes *m_boxes;
    PokeTableModel *pokeModel;
    Pokedex *m_pokedex;
    QAction *gens[NUMBER_GENS];
    int gen() const;

    QImageButton *buttons[LastW];
    QLabel *currentZoneLabel;
    /* the Team of the trainer */
    TrainerTeam *m_team;

    QPointer<QWidget> m_import;

    void updateTrainer();
    void updateTeam();
    void updateBox();

    void initBox();
    void initPokedex();
    void initTeam();

    TB_TrainerBody *trainerbody();

    bool modified[6];
};

#endif // TEAMBUILDER_H
