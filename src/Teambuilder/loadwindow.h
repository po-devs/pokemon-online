#ifndef LOADWINDOW_H
#define LOADWINDOW_H

#include <QDialog>

#include "Teambuilder/teamholder.h"

class LoadLine;

namespace Ui {
class LoadWindow;
}

class LoadWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoadWindow(QWidget *parent = 0, const QStringList &tierList = (QStringList() << " "));
    ~LoadWindow();
    
signals:
    void teamLoaded(const TeamHolder &t);

public slots:
    void onAccept();

private:
    Ui::LoadWindow *ui;
    LoadLine* lines[6];

    TeamHolder holder;
};

#endif // LOADWINDOW_H
