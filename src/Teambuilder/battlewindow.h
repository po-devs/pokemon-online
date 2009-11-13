#ifndef BATTLEWINDOW_H
#define BATTLEWINDOW_H

#include <QtGui>

class BattleWindow : public QWidget
{
    Q_OBJECT
public:
    BattleWindow();

    /* analyzes the command and calls the right function */
    void dealWithCommand(const QByteArray &);
signals:
    void battleCommand(const QByteArray &);
private:

};


#endif // BATTLEWINDOW_H
