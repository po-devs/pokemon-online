﻿#ifndef SCRIPTWINDOW_H
#define SCRIPTWINDOW_H

#include <QDialog>

namespace Ui {
class ScriptWindow;
}

class ScriptWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit ScriptWindow(QWidget *parent = 0);
    ~ScriptWindow();
    
    void accept();
signals:
    void scriptChanged(const QString&);
    void safeScriptsChanged(bool safe);
    void warningsChanged(bool warn);
public slots:
    void safeScriptsChanged(int newStatus);
    void warningsChanged(int newStatus);
private:
    Ui::ScriptWindow *ui;
};

#endif // SCRIPTWINDOW_H
