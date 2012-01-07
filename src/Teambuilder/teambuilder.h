#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QWidget>
#include "centralwidget.h"

class TeamHolder;
class TrainerMenu;

class TeamBuilder : public QStackedWidget, public CentralWidgetInterface
{
    Q_OBJECT
public:
    TeamBuilder(TeamHolder *team);
    ~TeamBuilder();

    virtual QSize defaultSize() const;
    virtual QMenuBar *createMenuBar(MainEngine *);
public slots:
    void saveAll();
    void loadAll();
    void newTeam();
signals:
    void done();
private:
    TeamHolder &team() {return *m_team;}
    TeamHolder *m_team;
    TrainerMenu *trainer;
};

#endif // TEAMBUILDER_H
