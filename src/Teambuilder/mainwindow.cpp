#include "mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "menu.h"
#include "teambuilder.h"
#include "client.h"
#include "serverchoice.h"
#include "../PokemonInfo/movesetchecker.h"
#include <QStyleFactory>

MainEngine::MainEngine() : displayer(0)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QSettings settings;
    /* initializing the default init values if not there */
    if (settings.value("new_teambuilder").isNull() || settings.value("application_style").isNull()) {
        settings.setValue("application_style", "plastique");
        settings.setValue("new_teambuilder",true);
    }
    if (settings.value("stylesheet").isNull() || settings.value("stylesheet").toString() == "db/default.qss") {
        settings.setValue("stylesheet", "db/default.css");
    }


    setDefaultValue("team_location", "Team/trainer.tp");
    setDefaultValue("save_battle_logs", false);
    setDefaultValue("battle_logs_directory", "Logs/");
    setDefaultValue("battle_music_directory", "Music/");
    setDefaultValue("play_battle_music", true);
    //setDefaultValue("play_battle_sounds", true);
    setDefaultValue("show_team",true);
    setDefaultValue("enable_ladder", true);
    setDefaultValue("show_player_events", false);
    setDefaultValue("show_timestamps", true);
    setDefaultValue("animate_hp_bar", true);

    setDefaultValue("find_battle_force_rated", false);
    setDefaultValue("find_battle_same_tier", true);
    setDefaultValue("find_battle_range_on", true);
    setDefaultValue("find_battle_range", 300);


    PokemonInfo::init("db/pokes/");
    MoveSetChecker::init("db/pokes/");
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    GenderInfo::init("db/genders/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");

    /* Loading the values */
    QApplication::setStyle(settings.value("application_style").toString());
    QFile stylesheet(settings.value("stylesheet").toString());
    stylesheet.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(stylesheet.readAll());
    loadTeam(settings.value("team_location").toString());

    launchMenu();
}

void MainEngine::loadStyleSheet()
{
    QSettings s;
    QFile stylesheet(s.value("stylesheet").toString());
    stylesheet.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(stylesheet.readAll());
}

#define MainEngineRoutine(widget) \
    displayer->deleteLater(); \
    displayer = new QMainWindow(); \
    displayer->resize(widget->size()); \
    displayer->setWindowTitle(tr("Pokemon Online")); \
    displayer->setCentralWidget(widget);\
    displayer->setMenuBar(widget->createMenuBar(this));\
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
    QFile fichier("db/credits.html");
    if(!fichier.open(QIODevice::ReadOnly))
    {
        return;
    }
    QDialog d_credit;
    d_credit.setMaximumSize(800,600);
    QVBoxLayout * l = new QVBoxLayout();
    QLabel * credit = new QLabel();
    //credit->setMaximumSize(800,600);
    l->addWidget(credit);
    credit->setAttribute(Qt::WA_DeleteOnClose,true);
    QTextStream out(&fichier);
    credit->setText(out.readAll());
    //MainEngineRoutine(d_credit);
    d_credit.setLayout(l);
    d_credit.move(this->displayer->geometry().x(),this->displayer->geometry().y());
    d_credit.setStyleSheet("background: qradialgradient(cx:0.5, cy:0.5, radius: 0.8,"
                                                       "stop:0 white, stop:1 #0ca0dd);");
    d_credit.exec();
}

void MainEngine::launchTeamBuilder()
{
    TeamBuilder *TB = new TeamBuilder(trainerTeam());
    MainEngineRoutine(TB);

    connect(TB,SIGNAL(showDock(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)),
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
    connect(choice, SIGNAL(serverChosen(QString,quint16)), this, SLOT(goOnline(QString,quint16)));
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

void MainEngine::goOnline(const QString &url, const quint16 port)
{
    Client * client = new Client(trainerTeam(), url, port);
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
