#ifndef TEAMIMPORTER_H
#define TEAMIMPORTER_H

#include <QWidget>

class QPlainTextEdit;

class TeamImporter : public QWidget
{
    Q_OBJECT
public:
    TeamImporter(QWidget *parent=0);
signals:
    void done(const QString&);
public slots:
    void done();
private:
    QPlainTextEdit *mycontent;
};

#endif // TEAMIMPORTER_H
