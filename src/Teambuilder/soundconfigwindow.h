#ifndef SOUNDCONFIGWINDOW_H
#define SOUNDCONFIGWINDOW_H

#include <QString>
#include <QWidget>

class SoundConfigWindow : public QObject
{
    Q_OBJECT
public:
    SoundConfigWindow();
public slots:
    void saveChanges();
private:
    int cryVolume;
    int musicVolume;
    bool playMusic, playCries;

    QString musicPath;
};

#endif // SOUNDCONFIGWINDOW_H
