#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QWidget>
#include "centralwidget.h"
#include "teambuilderwidget.h"

class TeamHolder;
class TrainerMenu;
class TeamMenu;

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
    void editPoke(int);
    void switchToTrainer();
private slots:
    void markTeamUpdated();
signals:
    void done();
private:
    TeamHolder &team() {return *m_team;}
    TeamHolder *m_team;
    TrainerMenu *trainer;
    TeamMenu *teamMenu;

    void markAllUpdated();
    void switchTo(TeamBuilderWidget *w);
};

#endif // TEAMBUILDER_H
