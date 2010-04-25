#include "menu.h"
#include "mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include <QStyleFactory>

TB_Menu::TB_Menu()
{
    setPixmap(QPixmap("db/menu/menu_background.png"));
    setWindowTitle(tr("Menu"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QImageButton *teambuilder, *online, *credits, *exit;

    layout->setMargin(0);
    layout->setSpacing(0);

    layout->addWidget(teambuilder = new QImageButton("db/menu/Teambuilder0.png", "db/menu/Teambuilder1.png"), 0, Qt::AlignCenter);
    layout->addWidget(online = new QImageButton("db/menu/GoOnline0.png", "db/menu/GoOnline1.png"), 0, Qt::AlignCenter);
    layout->addWidget(credits = new QImageButton("db/menu/Credits0.png", "db/menu/Credits1.png"), 0, Qt::AlignCenter);
    layout->addWidget(exit = new QImageButton("db/menu/Quit0.png", "db/menu/Quit1.png"), 0, Qt::AlignCenter);

    connect (teambuilder, SIGNAL(clicked()), SIGNAL(goToTeambuilder()));
    connect (online, SIGNAL(clicked()), SIGNAL(goToOnline()));
    connect (credits, SIGNAL(clicked()), SIGNAL(goToCredits()));
    connect (exit, SIGNAL(clicked()), SIGNAL(goToExit()));
}

QMenuBar * TB_Menu::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    menuBar->setStyleSheet("QMenuBar{background-image:url(db/menu/menu_background.png);}");
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&Load Team"),w,SLOT(loadTeamDialog()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("&Quit"),qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);
    QMenu *menuStyle = menuBar->addMenu(tr("&Style"));
    QStringList style = QStyleFactory::keys();
    for(QStringList::iterator i = style.begin();i!=style.end();i++)
    {
        menuStyle->addAction(*i,w,SLOT(changeStyle()));
    }
    QMenu *langMenu = menuBar->addMenu(tr("&Language"));
    QFile in ("languages.txt");
    in.open(QIODevice::ReadOnly);
    QStringList langs = QString::fromUtf8(in.readAll()).split('\n');
    foreach(QString a, langs) {
        langMenu->addAction(a,w, SLOT(changeLanguage()));
    }

    return menuBar;
}
