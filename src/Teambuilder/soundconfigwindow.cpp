#ifndef SOUNDCONFIGWINDOW_CPP
#define SOUNDCONFIGWINDOW_CPP

#include "soundconfigwindow.h"
#include "../Utilities/confighelper.h"

SoundConfigWindow::SoundConfigWindow()
{
    QSettings s;

    cryVolume = s.value("battle_cry_volume").toInt();
    musicVolume = s.value("battle_music_volume").toInt();
    musicPath = s.value("battle_music_directory").toString();
    playMusic = s.value("play_battle_music", false).toBool();
    playCries = s.value("play_battle_sounds").toBool();

    ConfigForm *form = new ConfigForm(tr("&Apply sound settings"), "", this);

    QString message = tr("If you become stuck in your battles and have problems when pokemon faint, <b>disable the pokemon cries.</b>");
#ifdef _WIN32
    message = QString("%1<br><br>%2").arg(message, tr("Download the <a href=\"http://www.codecguide.com/download_kl.htm\">K-Lite"
                                                                 " Codec pack</a> if you want Pokemon online to be able to read all your music!"));
#endif
    form->addConfigHelper(new AbstractConfigHelper("<div>"+message+"</div>"));

    form->addConfigHelper(new ConfigFile(tr("Music path: "), musicPath));
    form->addConfigHelper(new ConfigCheck(tr("Play battle music"), playMusic));
    form->addConfigHelper(new ConfigSlider(tr("Music volume: "), musicVolume, 0, 100));
    form->addConfigHelper(new ConfigCheck(tr("Play pokemon cries"), playCries));
    form->addConfigHelper(new ConfigSlider(tr("Cries volume: "), cryVolume, 0, 100));

    QWidget *w = form->generateConfigWidget();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->layout()->setMargin(15);
    w->setWindowTitle(tr("Sound config window"));
    w->resize(500, 400);
    w->show();

    connect(w, SIGNAL(destroyed()), SLOT(deleteLater()));

    connect(form, SIGNAL(button1()), form, SLOT(applyVals()));
    connect(form, SIGNAL(button1()), this, SLOT(saveChanges()));
    connect(form, SIGNAL(button1()), w, SLOT(deleteLater()));
}

void SoundConfigWindow::saveChanges()
{
    QSettings s;

    s.setValue("battle_cry_volume", cryVolume);
    s.setValue("battle_music_volume", musicVolume);
    s.setValue("battle_music_directory", musicPath);
    s.setValue("play_battle_music", playMusic);
    s.setValue("play_battle_sounds", playCries);

    emit cryVolumeChanged(cryVolume);
    emit musicVolumeChanged(musicVolume);
}

#endif // SOUNDCONFIGWINDOW_CPP
