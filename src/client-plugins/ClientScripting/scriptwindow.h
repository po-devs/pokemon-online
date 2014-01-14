#ifndef SCRIPTWINDOW_H
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
    void battleScriptChanged(const QString&);
    void safeScriptsChanged(bool safe);
    void warningsChanged(bool warn);
    void moogleScripts();
public slots:
    void safeScriptsChanged(int newStatus);
    void warningsChanged(int newStatus);
    void setScripts(const QString &scripts);
    void setOffline();
private slots:
    void on_moogle_clicked();
private:
    Ui::ScriptWindow *ui;
};

#endif // SCRIPTWINDOW_H
