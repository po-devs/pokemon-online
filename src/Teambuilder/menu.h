#ifndef MENU_H
#define MENU_H

#include <QFrame>
#include "centralwidget.h"

namespace Ui {
class Menu;
}

class MainEngine;
class TeamHolder;

class Menu : public QFrame, public CentralWidgetInterface
{
    Q_OBJECT
    
public:
    explicit Menu(TeamHolder *t);
    ~Menu();

    /* Creates a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainEngine *w);

    void showChangeLog();
    void disableUpdateButton();
signals:
    void goToTeambuilder();
    void goToOnline();
    void goToCredits();
    void goToExit();

    void downloadUpdateRequested();
public slots:
    void loadTeam();
    void loadAll(const TeamHolder&);

    void setUpdateData(const QString &data);
    void setChangeLogData(const QString &data);

    void prevTip();
    void nextTip();
private slots:
    void on_updateButton_clicked();
protected:
    bool event(QEvent *e);
private:
    Ui::Menu *ui;

    TeamHolder *team;
    QStringList msgs;
    int currentTip;

    void updateTip();
};

#endif // MENU_H
