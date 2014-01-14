#ifndef LOADWINDOW_H
#define LOADWINDOW_H

#include <QDialog>
#include <QColorDialog>
#include <QSettings>

#include <Utilities/functions.h>
#include <PokemonInfo/teamholder.h>

class LoadLine;

namespace Ui {
class LoadWindow;
}

class LoadWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoadWindow(QWidget *parent = 0, const QStringList &tierList = QStringList(), const QString &name = "");
    ~LoadWindow();
    
signals:
    void teamLoaded(const TeamHolder &t);

public slots:
    void onAccept();

private slots:
    void on_colorButton_clicked();
    void setColor(QColor c);

    void on_profileList_currentIndexChanged(const QString &arg1);

private:
    Ui::LoadWindow *ui;
    LoadLine* lines[6];

    TeamHolder holder;
};

#endif // LOADWINDOW_H
