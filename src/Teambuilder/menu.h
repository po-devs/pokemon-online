#ifndef MENU_H
#define MENU_H

#include <QFrame>
#include "centralwidget.h"

namespace Ui {
class Menu;
}

class MainEngine;

class Menu : public QFrame, public CentralWidgetInterface
{
    Q_OBJECT
    
public:
    explicit Menu(QWidget *parent = 0);
    ~Menu();

    /* Creates a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainEngine *w);

signals:
    void goToTeambuilder();
    void goToOnline();
    void goToCredits();
    void goToExit();

protected:

private:
    Ui::Menu *ui;
};

#endif // MENU_H
