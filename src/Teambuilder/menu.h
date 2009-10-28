#ifndef TEAMBUILDER_MENU_H
#define TEAMBUILDER_MENU_H

#include <QtGui>

class MainWindow;

class TB_Menu : public QWidget
{
        Q_OBJECT
public:
    TB_Menu();
    /* Creates a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainWindow *w);

public slots:
    void launchTeambuilder();
    void launchCredits();
    void goOnline();
    void exit();
signals:
    void goToTeambuilder();
    void goToOnline();
    void goToCredits();
    void goToExit();
};

#endif
