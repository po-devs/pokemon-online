#ifndef CORE_H
#define CORE_H

#include <QThread>
#include <QSemaphore>

class Core : public QThread
{
    Q_OBJECT
public:
    explicit Core(QObject *parent = 0);
    void setSource(const QString &source);
    
    void skipFile();
    void restart();

    void stop();

    void run();
signals:
    void progress(const QString& progress);
    /* There's been a problem with the file. The thread pauses,
      and the main class decides to abort (stop), skip (skipFile+restart),
      or retry (restart) */
    void problem(const QString &file);
public slots:
private:
    QString source;

    void recurseUpdate(const QString &dir);

    QSemaphore sem;

    bool quit;
    bool skip;

    void pause();
};

#endif // CORE_H
