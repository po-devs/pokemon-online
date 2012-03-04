#ifndef TEAMBUILDERWIDGET_H
#define TEAMBUILDERWIDGET_H

#include <QFrame>

class TeamBuilderWidget  : public QFrame
{
    Q_OBJECT
public:
    TeamBuilderWidget(QWidget *parent=0) : QFrame(parent){}
    virtual void updateAll() {updateTeam();}
    virtual void updateTeam(){}
signals:
    void teamChanged();
};

#endif // TEAMBUILDERWIDGET_H
