#include "menu.h"
#include "theme.h"
#include "mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include <QStyleFactory>


TB_Menu::TB_Menu()
{
    setPixmap(Theme::Sprite("menubackground"));
    setWindowTitle(tr("Menu"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    QImageButtonP *credits,*teambuilder, *online, *exit;

    layout->setMargin(0);
    layout->setSpacing(0);

    layout->addWidget(teambuilder = Theme::PressedButton("teambuilder"), 0, Qt::AlignCenter);
    teambuilder->setAccessibleName(tr("Teambuilder"));
    layout->addWidget(online = Theme::PressedButton("goonline"), 0, Qt::AlignCenter);
    online->setAccessibleName(tr("Go online"));
    layout->addWidget(credits = Theme::PressedButton("credits"), 0, Qt::AlignCenter);
    credits->setAccessibleName(tr("Credits"));
    layout->addWidget(exit = Theme::PressedButton("quit"), 0, Qt::AlignCenter);
    exit->setAccessibleName(tr("exit"));

    connect (teambuilder, SIGNAL(clicked()), SIGNAL(goToTeambuilder()));
    connect (online, SIGNAL(clicked()), SIGNAL(goToOnline()));
    connect (credits, SIGNAL(clicked()), SIGNAL(goToCredits()));
    connect (exit, SIGNAL(clicked()), SIGNAL(goToExit()));
}

QMenuBar * TB_Menu::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&Load Team"),w,SLOT(loadTeamDialog()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("Open &replay"),w,SLOT(loadReplayDialog()), Qt::CTRL+Qt::Key_R);
    menuFichier->addAction(tr("&Quit"),qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);

    w->addStyleMenu(menuBar);
    w->addThemeMenu(menuBar);

    QMenu *langMenu = menuBar->addMenu(tr("&Language"));
    QFile in ("languages.txt");
    in.open(QIODevice::ReadOnly);
    QStringList langs = QString::fromUtf8(in.readAll()).split('\n');
    foreach(QString a, langs) {
        langMenu->addAction(a,w, SLOT(changeLanguage()));
    }

    return menuBar;
}
