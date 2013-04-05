#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include "../PokemonInfo/pokemon.h"
#include "../PokemonInfo/geninfo.h"
#include <QWidget>
#include "centralwidget.h"
#include "Teambuilder/teambuilderwidget.h"
#include "teambuilderinterface.h"
#include "plugininterface.h"

class TeamHolder;
class TrainerMenu;
class TeamMenu;
class PokeBoxes;
class PluginManager;

class TeamBuilder : public QMainWindow, public TeambuilderInterface, public CentralWidgetInterface
{
    Q_OBJECT
public:
    TeamBuilder(PluginManager *p, TeamHolder *team, bool loadSettings=true);
    ~TeamBuilder();

    virtual QSize defaultSize() const;
    virtual QMenuBar *createMenuBar(MainEngine *);

    void addPlugin(TeambuilderPlugin *o);
    void removePlugin(TeambuilderPlugin *o);

    TeamBuilderWidget *currentWidget();
    TeamBuilderWidget *widget(int i);
public slots:
    void saveAll();
    void openLoadWindow();
    void loadAll(const TeamHolder &t);
    void newTeam();
    void openBoxes();
    void editPoke(int);
    void switchToTrainer();
    void setTierList(const QStringList &tiers); //tells which tiers are available

    void setNoMod();
    void changeMod();
    void genChanged();
    void installMod();
    void removeMods();
    void removeMod();

    void importTeam();
    void exportTeam();
    void addTeam();
    void openTeam();
    void saveTeam();
private slots:
    void markTeamUpdated();
    void updateCurrentTeamAndNotify();
    void onSaveTeam();
signals:
    void done();
    void reloadMenuBar();
    void reloadDb();
private:
    QListWidget *modsList;
    TeamHolder &team() {return *m_team;}
    TeamHolder *m_team;
    TrainerMenu *trainer;
    TeamMenu *teamMenu;

    QSet<TeambuilderPlugin*> plugins;
    PluginManager *pluginManager;
    QHash<TeambuilderPlugin*, QHash<QString, TeambuilderPlugin::Hook> > hooks;

    QAbstractItemModel *pokemonModel;

    struct _ui {
        QHash<Pokemon::gen, QAction*> gens;
        QStackedWidget *stack;
    };

    Pokemon::gen lastGen;

    _ui * ui;

    void markAllUpdated();
    void switchTo(TeamBuilderWidget *w);
public:
    /* Call a plugin function */
    template<class T1>
    bool call(const QString &f, T1 arg1)
    {
        bool ret = true;
        foreach(TeambuilderPlugin *p, plugins) {
            if (hooks[p].contains(f)) {
                ret &= (*p.*(reinterpret_cast<int (TeambuilderPlugin::*)(T1)>(hooks[p][f])))(arg1);
            }
        }

        return ret;
    }

    template<class T1, class T2>
    bool call(const QString &f, T1 arg1, T2 arg2)
    {
        bool ret = true;
        foreach(TeambuilderPlugin *p, plugins) {
            if (hooks[p].contains(f)) {
                ret &= (*p.*(reinterpret_cast<int (TeambuilderPlugin::*)(T1, T2)>(hooks[p][f])))(arg1, arg2);
            }
        }

        return ret;
    }

    template<class T1, class T2, class T3>
    bool call(const QString &f, T1 arg1, T2 arg2, T3 arg3)
    {
        bool ret = true;
        foreach(TeambuilderPlugin *p, plugins) {
            if (hooks[p].contains(f)) {
                ret &= (*p.*(reinterpret_cast<int (TeambuilderPlugin::*)(T1, T2, T3)>(hooks[p][f])))(arg1, arg2, arg3);
            }
        }

        return ret;
    }

    template<class T1, class T2, class T3, class T4>
    bool call(const QString &f, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
    {
        bool ret = true;
        foreach(TeambuilderPlugin *p, plugins) {
            if (hooks[p].contains(f)) {
                ret &= (*p.*(reinterpret_cast<int (TeambuilderPlugin::*)(T1, T2, T3, T4)>(hooks[p][f])))(arg1, arg2, arg3, arg4);
            }
        }

        return ret;
    }
};

#endif // TEAMBUILDER_H
