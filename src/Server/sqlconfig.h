#ifndef SQLCONFIG_H
#define SQLCONFIG_H

#include <QtGui>

class SQLConfigWindow : public QWidget
{
    Q_OBJECT
public:
    SQLConfigWindow();

public slots:
    void apply();
    void changeEnabled();
private:
    QComboBox *b;
    QLineEdit *host, *name, *user, *pass;
    QSpinBox *port;
};

#endif // SQLCONFIG_H
