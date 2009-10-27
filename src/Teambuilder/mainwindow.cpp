#include "mainwindow.h"

#include "../PokemonInfo/pokemoninfo.h"
#include "menu.h"
#include "teambuilder.h"

MainWindow::MainWindow() : m_menu(0), m_TB(0)
{
    resize (650, 680);
    setWindowTitle(tr("Pogeymon-Online"));

    centralZone = new QMdiArea;
    setCentralWidget(centralZone);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    PokemonInfo::init("db/");
    ItemInfo::init("db/");
    MoveInfo::init("db/");
    TypeInfo::init("db/");
    NatureInfo::init("db/");
    CategoryInfo::init("db/");
    AbilityInfo::init("db/");
    GenderInfo::init("db/");
    HiddenPowerInfo::init("db/");

    launchMenu();
}

TrainerTeam * MainWindow::trainerTeam()
{
    return & m_team;
}

void MainWindow::launchMenu()
{
    m_menu = new TB_Menu();

    menuSubWindow = centralZone->addSubWindow(m_menu);
    
    connect(menuSubWindow, SIGNAL(destroyed()), SLOT(windowDestroyed()));

    menuSubWindow->show();

    menuSubWindow->move((width() - menuSubWindow->width())/2, (height() - menuSubWindow->height())/2);

    connect(m_menu, SIGNAL(goToTeambuilder()), SLOT(launchTeamBuilder()));
    connect(m_menu, SIGNAL(goToExit()), SLOT(close()));
    connect(m_menu, SIGNAL(goToOnline()), SLOT(goOnline()));
    connect(m_menu, SIGNAL(goToCredits()), SLOT(launchCredits()));

    if (m_TB && sender() == m_TB) {
        delete TBSubWindow, m_TB=NULL;
    }
}

void MainWindow::launchCredits()
{
}

void MainWindow::launchTeamBuilder()
{
    m_TB = new TeamBuilder(trainerTeam());

    TBSubWindow = centralZone->addSubWindow(m_TB);

    connect(TBSubWindow, SIGNAL(destroyed()), SLOT(windowDestroyed()));
    
    m_TB->show();

    connect(m_TB, SIGNAL(done()), SLOT(launchMenu()));

    if (m_menu && sender() == m_menu) {
        delete menuSubWindow, m_menu = NULL;
    }
}

void MainWindow::windowDestroyed()
{
    if (sender() == TBSubWindow) {
        m_TB=NULL;

        if (centralZone->subWindowList().isEmpty()) {
            launchMenu();
        }
    } else if (sender() == menuSubWindow) {
        m_menu = NULL;

        if (centralZone->subWindowList().isEmpty()) {
            close();
        }
    }
}

void MainWindow::goOnline()
{
}
