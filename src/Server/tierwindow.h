#ifndef TIERWINDOW_H
#define TIERWINDOW_H

#include <QtGui>

class TierWindow : public QWidget
{
    Q_OBJECT
public:
    TierWindow(QWidget *parent = NULL);
signals:
    void tiersChanged();
private slots:
    void done();
private:
    QPlainTextEdit *m_editWindow;
};


#endif // TIERWINDOW_H
