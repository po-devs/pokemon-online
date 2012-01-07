#ifndef TEAMMENU_H
#define TEAMMENU_H

#include "teambuilderwidget.h"

class QStackedWidget;
class QTabBar;

class TeamMenu : public TeamBuilderWidget
{
    Q_OBJECT

public:
    explicit TeamMenu(QWidget *parent = 0);
    ~TeamMenu();
private:
    void setupUi();

    struct _ui {
        QStackedWidget *stack;
        QTabBar *pokemonTabs;
    };

    _ui *ui;
};

#endif // TEAMMENU_H
