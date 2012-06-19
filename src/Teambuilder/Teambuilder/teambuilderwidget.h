#ifndef TEAMBUILDERWIDGET_H
#define TEAMBUILDERWIDGET_H

#include <QFrame>

class QMenuBar;

class TeamBuilderWidget  : public QFrame
{
    Q_OBJECT
public:
    TeamBuilderWidget(QWidget *parent=0) : QFrame(parent){}
    virtual void updateAll() {updateTeam();}
    virtual void updateTeam(){}
    virtual void addMenus(QMenuBar *){}
signals:
    void teamChanged();
};

#endif // TEAMBUILDERWIDGET_H
