#ifndef TEAMBUILDER_MENU_H
#define TEAMBUILDER_MENU_H

#include <QtGui>
#include "centralwidget.h"

class MainEngine;

/* The plain menu you see when the program starts.
   When you click a button, it sends a signal to the main window to tell it to
   change the module displayed

    There are 4 buttons:
    -Teambuilder
    -Go Online
    -See Credits
    -Exit

    To make it classier, the whole menu is just images,
    and the background too (thus its inheritance to QImageBackground)

 */

class TB_Menu : public QLabel, public CentralWidgetInterface
{
        Q_OBJECT
public:
    TB_Menu();
    /* Creates a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainEngine *w);

signals:
    void goToTeambuilder();
    void goToOnline();
    void goToCredits();
    void goToExit();
};

#endif
