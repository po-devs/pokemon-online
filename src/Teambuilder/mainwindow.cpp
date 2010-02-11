#include "mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "menu.h"
#include "teambuilder.h"
#include "client.h"
#include "serverchoice.h"
#include <QStyleFactory>

MainWindow::MainWindow() : m_menu(0), m_TB(0)
{
    this->setObjectName("MainWindow");

    setWindowTitle(tr("Pokemon Online"));
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QSettings settings;
    /* initializing the default init values if not there */
    if (settings.value("team_location").isNull()) {
	settings.setValue("team_location", "Team/trainer.tp");
    }
    if (settings.value("application_style").isNull()) {
	settings.setValue("application_style", "cleanlooks");
    }
    if (settings.value("stylesheet").isNull()) {
	settings.setValue("stylesheet", "db/default.qss");
    }

    PokemonInfo::init("db/");
    ItemInfo::init("db/");
    MoveInfo::init("db/");
    TypeInfo::init("db/");
    NatureInfo::init("db/");
    CategoryInfo::init("db/");
    AbilityInfo::init("db/");
    GenderInfo::init("db/");
    HiddenPowerInfo::init("db/");
    StatInfo::init("db/");

    /* Loading the values */
    QApplication::setStyle(settings.value("application_style").toString());
    QFile stylesheet(settings.value("stylesheet").toString());
    stylesheet.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(stylesheet.readAll());
    loadTeam(settings.value("team_location").toString());
    /* launching the first window */
    launchMenu();
}

TrainerTeam * MainWindow::trainerTeam()
{
    return & m_team;
}

void MainWindow::launchMenu()
{
    m_menu = new TB_Menu();

    /* We want to have space around the menu, so we put it in another widget ... */

    setCentralWidget(m_menu);
    setMenuBar(m_menu->createMenuBar(this));

    connect(m_menu, SIGNAL(goToTeambuilder()), SLOT(launchTeamBuilder()));
    connect(m_menu, SIGNAL(goToExit()), SLOT(close()));
    connect(m_menu, SIGNAL(goToOnline()), SLOT(launchServerChoice()));
    connect(m_menu, SIGNAL(goToCredits()), SLOT(launchCredits()));
}

void MainWindow::launchCredits()
{
}

void MainWindow::launchTeamBuilder()
{
    m_TB = new TeamBuilder(trainerTeam());
    connect(m_TB,SIGNAL(showDockAdvanced(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)),
            this,SLOT(setDock(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)));
    connect(m_TB, SIGNAL(done()), SLOT(launchMenu()));

    setCentralWidget(m_TB);
    setMenuBar(m_TB->createMenuBar(this));
}

void MainWindow::launchServerChoice()
{
    m_choice = new ServerChoice();

    connect(m_choice, SIGNAL(rejected()), SLOT(launchMenu()));
    connect(m_choice, SIGNAL(serverChosen(QString)), this, SLOT(goOnline(QString)));

    setMenuBar(NULL);
    setCentralWidget(m_choice);
}

void MainWindow::changeStyle()
{
    QAction * a = qobject_cast<QAction *>(sender());
    if(!a)
    {
        return;
    }
    QString style = a->text();
    qApp->setStyle(QStyleFactory::create(style));
    QSettings setting;
    setting.setValue("application_style",style);
}

void MainWindow::changeLanguage()
{
    QAction * a = qobject_cast<QAction *>(sender());
    if(!a)
    {
        return;
    }

    QString lang = a->text().split('(').back().left(2);
    QSettings setting;

    if (setting.value("language").toString() == lang) {
        return;
    }

    setting.setValue("language",lang);

    QMessageBox::information(this, tr("Language Change"), tr("Restart the application to the changes."));
}

void MainWindow::goOnline(const QString &url)
{
    m_client = new Client(trainerTeam(), url);
    setCentralWidget(m_client);
    setMenuBar(m_client->createMenuBar(this));

    connect(m_client, SIGNAL(done()), SLOT(launchMenu()));
}

void MainWindow::loadTeam(const QString &path)
{
    trainerTeam()->loadFromFile(path);
}

void MainWindow::loadTeamDialog()
{
    QSettings settings;
    QString newLocation;

    if (loadTTeamDialog(*trainerTeam(), settings.value("team_location").toString(), &newLocation)) {
        settings.setValue("team_location", newLocation);
    }
}

void MainWindow::setDock(Qt::DockWidgetArea areas,QDockWidget * dock,Qt::Orientation orient)
{
    this->addDockWidget(areas,dock,orient);
}

void MainWindow::removeDock(QDockWidget * dock)
{
    this->removeDockWidget(dock);
}
