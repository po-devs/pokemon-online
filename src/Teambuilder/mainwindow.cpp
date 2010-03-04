#include "mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "menu.h"
#include "teambuilder.h"
#include "client.h"
#include "serverchoice.h"
#include <QStyleFactory>

MainEngine::MainEngine() : displayer(0)
{
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
    if (settings.value("save_battle_logs").isNull()) {
        settings.setValue("save_battle_logs", false);
    }
    if (settings.value("battle_logs_directory").isNull()) {
        settings.setValue("battle_logs_directory", "Logs/");
    }
    if (settings.value("show_team").isNull()) {
        settings.setValue("show_team", true);
    }
    if (settings.value("enable_ladder").isNull()) {
        settings.setValue("enable_ladder",true);
    }
    if (settings.value("show_player_events").isNull()) {
        settings.setValue("show_player_events", false);
    }

    //trainerTeam()->setTrainerNick(settings.value("trainer_name").toString());

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

    launchMenu();
}

#define MainEngineRoutine(widget) \
    delete displayer; \
    displayer = new QMainWindow(); \
    displayer->resize(widget->size()); \
    displayer->setWindowTitle(tr("Pokemon Online")); \
    displayer->setCentralWidget(widget); \
    displayer->setMenuBar(widget->createMenuBar(this)); \
    displayer->show();

void MainEngine::launchMenu()
{
    TB_Menu *menu = new TB_Menu();
    MainEngineRoutine(menu);
    displayer->layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(menu, SIGNAL(goToTeambuilder()), SLOT(launchTeamBuilder()));
    connect(menu, SIGNAL(goToExit()), SLOT(quit()));
    connect(menu, SIGNAL(goToOnline()), SLOT(launchServerChoice()));
    connect(menu, SIGNAL(goToCredits()), SLOT(launchCredits()));
}

void MainEngine::launchCredits()
{
}

void MainEngine::launchTeamBuilder()
{
    TeamBuilder *TB = new TeamBuilder(trainerTeam());
    MainEngineRoutine(TB);

    connect(TB,SIGNAL(showDockAdvanced(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)),
            SLOT(setDock(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)));
    connect(TB, SIGNAL(done()), SLOT(launchMenu()));
}

void MainEngine::launchServerChoice()
{
    if (trainerTeam()->trainerNick().length() == 0) {
        QMessageBox::information(displayer, tr("Impossible to go online"), tr("You haven't set your name yet. Do so in the teambuilder."));
        return;
    }

    ServerChoice *choice = new ServerChoice();
    MainEngineRoutine(choice);

    connect(choice, SIGNAL(rejected()), SLOT(launchMenu()));
    connect(choice, SIGNAL(serverChosen(QString)), this, SLOT(goOnline(QString)));
}

void MainEngine::changeStyle()
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

void MainEngine::changeLanguage()
{
    QAction * a = qobject_cast<QAction *>(sender());
    if(!a)
    {
        return;
    }

    QString lang = a->text().split('(').back().split(')').front();
    QSettings setting;

    if (setting.value("language").toString() == lang) {
        return;
    }

    setting.setValue("language",lang);

    QMessageBox::information(displayer, tr("Language Change"), tr("Restart the application to see the changes."));
}

void MainEngine::goOnline(const QString &url)
{
    Client * client = new Client(trainerTeam(), url);
    MainEngineRoutine(client);

    connect(client, SIGNAL(done()), SLOT(launchMenu()));
}

void MainEngine::updateMenuBar()
{
    displayer->setMenuBar(((Client*)sender())->createMenuBar(this));
}

void MainEngine::quit()
{
    exit(0);
}

void MainEngine::loadTeam(const QString &path)
{
    trainerTeam()->loadFromFile(path);
}

void MainEngine::loadTeamDialog()
{
    QSettings settings;
    QString newLocation;

    if (loadTTeamDialog(*trainerTeam(), settings.value("team_location").toString(), &newLocation)) {
        settings.setValue("team_location", newLocation);
    }
}

void MainEngine::setDock(Qt::DockWidgetArea areas,QDockWidget * dock,Qt::Orientation orient)
{
    //displayer->resize(displayer->width() + dock->width(), displayer->height());
    displayer->addDockWidget(areas,dock,orient);
}

void MainEngine::removeDock(QDockWidget * dock)
{
    displayer->removeDockWidget(dock);
}

#undef MainEngineRoutine
