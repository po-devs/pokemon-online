#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QWidget>
#include "centralwidget.h"
#include "Teambuilder/teambuilderwidget.h"

class TeamHolder;
class TrainerMenu;
class TeamMenu;
class PokeBoxes;

class TeamBuilder : public QStackedWidget, public CentralWidgetInterface
{
    Q_OBJECT
public:
    TeamBuilder(TeamHolder *team);
    ~TeamBuilder();

    virtual QSize defaultSize() const;
    virtual QMenuBar *createMenuBar(MainEngine *);

    TeamBuilderWidget *currentWidget();
    TeamBuilderWidget *widget(int i);
public slots:
    void saveAll();
    void loadAll();
    void newTeam();
    void openBoxes();
    void editPoke(int);
    void switchToTrainer();
    void setTierList(const QStringList &tiers); //tells which tiers are available

    void setNoMod(){}
    void changeMod(){}

    void importTeam();
    void exportTeam();
private slots:
    void markTeamUpdated();
signals:
    void done();
    void reloadMenuBar();
private:
    TeamHolder &team() {return *m_team;}
    TeamHolder *m_team;
    TrainerMenu *trainer;
    TeamMenu *teamMenu;
    PokeBoxes *boxesMenu;

    QAbstractItemModel *pokemonModel;

    void markAllUpdated();
    void switchTo(TeamBuilderWidget *w);
};

#endif // TEAMBUILDER_H
