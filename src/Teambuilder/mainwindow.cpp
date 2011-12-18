#include "mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "menu.h"
#include "Teambuilder/teambuilder.h"
#include "client.h"
#include "serverchoice.h"
#include "../PokemonInfo/movesetchecker.h"
#include "pluginmanager.h"
#include "plugininterface.h"
#include "theme.h"
#include "../Utilities/functions.h"

MainEngine::MainEngine() : displayer(0)
{
    pluginManager = new PluginManager(this);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QSettings s;
    /* initializing the default init values if not there */
    setDefaultValue(s, "application_style", "plastique");
    setDefaultValue(s, "theme_2", "Themes/Dratini Dreams/");

#ifdef Q_OS_MACX
    setDefaultValue(s, "team_location", QDir::homePath() + "/Documents/trainer.tp");
#else
    setDefaultValue(s, "team_location", "Team/trainer.tp");
#endif
    setDefaultValue(s, "battle_music_directory", "Music/Battle/");
    setDefaultValue(s, "play_battle_music", false);
    setDefaultValue(s, "play_battle_sounds", false);
    setDefaultValue(s, "flash_when_enemy_moves", true);
    setDefaultValue(s, "show_team", true);
    setDefaultValue(s, "enable_ladder", true);
    setDefaultValue(s, "show_player_events_idle", false);
    setDefaultValue(s, "show_player_events_battle", false);
    setDefaultValue(s, "show_player_events_channel", false);
    setDefaultValue(s, "show_player_events_team", false);
    setDefaultValue(s, "show_timestamps", true);
    setDefaultValue(s, "show_timestamps2", true);
    setDefaultValue(s, "pm_flashing", true);
    setDefaultValue(s, "pm_disabled", false);
    setDefaultValue(s, "animate_hp_bar", true);
    setDefaultValue(s, "sort_players_by_tier", false);
    setDefaultValue(s, "sort_channels_by_name", false);
    setDefaultValue(s, "show_all_items", false);

    setDefaultValue(s, "find_battle_force_rated", false);
    setDefaultValue(s, "find_battle_same_tier", true);
    setDefaultValue(s, "find_battle_range_on", true);
    setDefaultValue(s, "find_battle_range", 200);

    if (s.value("use_socks5_proxy", false).toBool() == true) {
        s.beginGroup("socks5_proxy");
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::Socks5Proxy);
        proxy.setPort(s.value("port", 27977).toInt());
        proxy.setHostName(s.value("host").toString());
        proxy.setUser(s.value("user").toString());
        proxy.setPassword(s.value("pass").toString());
        s.endGroup();
        QNetworkProxy::setApplicationProxy(proxy);
    }

    QSettings s_mod(PoModLocalPath + "mods.ini", QSettings::IniFormat);
    QStringList mods = s_mod.childGroups();
    QString modname;

    if (mods.size() > 0) {
        int general_pos = mods.indexOf("General");
        if (general_pos != -1) {
            mods.removeAt(general_pos);
        }
        if (mods.size() > 0) {
            int mod_selected = s_mod.value("active", 0).toInt();
            bool is_mod_selected = mod_selected > 0;

            QStringListIterator mods_it(mods);
            while (mods_it.hasNext()) {
                QString current = mods_it.next();
                if (is_mod_selected && (mod_selected == s_mod.value(current + "/id", 0).toInt())) {
                    modname = current;
                }
            }
        }
    }

    PokemonInfo::init("db/pokes/", FillMode::Client, modname);
    MoveSetChecker::init("db/pokes/", s.value("enforce_min_levels").toBool());
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    GenderInfo::init("db/genders/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");
    Theme::init(s.value("theme_2").toString());

    /* Loading the values */
    QApplication::setStyle(s.value("application_style").toString());
    loadStyleSheet();
    loadTeam(s.value("team_location").toString());

    launchMenu();
}

MainEngine::~MainEngine()
{
    delete pluginManager, pluginManager = NULL;
}

QMenuBar *MainEngine::transformMenuBar(QMenuBar *param)
{
    if (param) {
        QMenu *m = param->addMenu(tr("Plugins"));
        m->addAction(tr("Plugin Manager"), this, SLOT(openPluginManager()));
        m->addSeparator();

        foreach(QString plugin, pluginManager->getVisiblePlugins()) {
            m->addAction(plugin, this, SLOT(openPluginConfiguration()));
        }
    }

    return param;
}

void MainEngine::openPluginManager()
{
    PluginManagerWidget *w = new PluginManagerWidget(*pluginManager);

    w->show();

    connect(w, SIGNAL(pluginListChanged()), SLOT(updateMenuBar()));
}

void MainEngine::openPluginConfiguration()
{
    QString plugin = ((QAction*)(sender()))->text();

    ClientPlugin *c = pluginManager->plugin(plugin);

    if (c && c->hasConfigurationWidget()) {
        QWidget *w = c->getConfigurationWidget();
        if (w) {
            w->setAttribute(Qt::WA_DeleteOnClose, true);
            w->show();
        }
    }
}

void MainEngine::loadStyleSheet()
{
    QFile stylesheet(Theme::path("default.css"));
    stylesheet.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(stylesheet.readAll());
}

#define MainEngineRoutine(widget) \
    if (displayer) \
        displayer->deleteLater(); \
    displayer = new QMainWindow(); \
    displayer->resize(widget->size()); \
    displayer->setWindowTitle(tr("Pokemon Online")); \
    displayer->setCentralWidget(widget);\
    displayer->setMenuBar(transformMenuBar(widget->createMenuBar(this)));\
    loadSettings(widget, widget->defaultSize());\
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
    d_credit.setMaximumSize(800,700);
    QVBoxLayout * l = new QVBoxLayout();
    QScrollArea *scroll = new QScrollArea();
    QLabel * credit = new QLabel();
    credit->setMargin(5);
    QTextStream out(&fichier);
    credit->setText(out.readAll());
    scroll->setWidget(credit);
    //credit->setMaximumSize(800,600);
    l->addWidget(scroll);
    scroll->show();
    credit->setAttribute(Qt::WA_DeleteOnClose,true);

    scroll->adjustSize();
    //MainEngineRoutine(d_credit);
    d_credit.setLayout(l);
    d_credit.move(this->displayer->geometry().x(),this->displayer->geometry().y());
    d_credit.setStyleSheet(
                "QWidget {background: qradialgradient(cx:0.5, cy:0.5, radius: 0.8,"
                                                       "stop:0 white, stop:1 #0ca0dd);}"
                "QLabel {background:transparent}"
                           );
    d_credit.exec();
}

void MainEngine::launchTeamBuilder()
{
    TeamBuilder *TB = new TeamBuilder(trainerTeam());
    MainEngineRoutine(TB);

    connect(TB, SIGNAL(done()), SLOT(launchMenu()));
}

void MainEngine::launchServerChoice()
{
    ServerChoice *choice = new ServerChoice(trainerTeam()->trainerNick());
    MainEngineRoutine(choice);

    connect(choice, SIGNAL(rejected()), SLOT(launchMenu()));
    connect(choice, SIGNAL(serverChosen(QString,quint16,QString)), this, SLOT(goOnline(QString,quint16,QString)));
}

void MainEngine::changeStyle()
{
    QAction * a = qobject_cast<QAction *>(sender());
    if(!a) {
        return;
    }
    QString style = a->text();
    qApp->setStyle(QStyleFactory::create(style));
    QSettings setting;
    setting.setValue("application_style",style);
}

void MainEngine::changeTheme()
{
    QAction * a = qobject_cast<QAction *>(sender());
    if(!a) {
        return;
    }
    QString theme = a->text();

    changeTheme(theme);
}

void MainEngine::changeTheme(const QString &theme)
{
    QSettings settings;

    QString fullTheme = "Themes/" + theme + "/";
    settings.setValue("theme_2", fullTheme);

    Theme::Reload(fullTheme);
    loadStyleSheet();
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

void MainEngine::goOnline(const QString &url, const quint16 port, const QString& nick)
{
    if (nick.size() > 0)
        trainerTeam()->setTrainerNick(nick);

    if (trainerTeam()->trainerNick().length() == 0) {
        QMessageBox::information(displayer, tr("Impossible to go online"), tr("You haven't set your name yet. Do so in the teambuilder."));
        return;
    }

    Client * client = new Client(trainerTeam(), url, port);
    MainEngineRoutine(client);

    connect(client, SIGNAL(done()), SLOT(launchMenu()));
}

void MainEngine::updateMenuBar()
{
    displayer->setMenuBar(transformMenuBar(dynamic_cast<CentralWidgetInterface*>(displayer->centralWidget())
                            ->createMenuBar(this)));
}

void MainEngine::quit()
{
    /* Has to be deleted here, otherwise windows error if the libraries are not detached */
    delete pluginManager, pluginManager = NULL;
    exit(0);
}

void MainEngine::loadTeam(const QString &path)
{
    trainerTeam()->loadFromFile(path);
}

void MainEngine::loadTeamDialog()
{
    loadTTeamDialog(*trainerTeam());
}

void MainEngine::addStyleMenu(QMenuBar *menuBar)
{
    QMenu * menuStyle = menuBar->addMenu(tr("&Style"));
    QStringList style = QStyleFactory::keys();
    QActionGroup *ag = new QActionGroup(menuBar);

    QSettings settings;
    QString curStyle = settings.value("application_style").toString();

    foreach(QString s , style) {
        QAction *ac = menuStyle->addAction(s,this,SLOT(changeStyle()));
        ac->setCheckable(true);

        if (s == curStyle) {
            ac->setChecked(true);
        }
        ag->addAction(ac);
    }

    menuStyle->addSeparator();
    menuStyle->addAction(tr("Reload StyleSheet"), this, SLOT(loadStyleSheet()));
}

void MainEngine::addThemeMenu(QMenuBar *menuBar)
{
    QMenu *themeMenu = menuBar->addMenu(tr("&Theme"));

    QDir d("Themes");

    QList<QFileInfo> dirs = d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    QSettings s;
    QString theme = s.value("theme_2").toString().section('/', -2, -2);

    QActionGroup *ag = new QActionGroup(themeMenu);
    foreach(QFileInfo f, dirs) {
        QAction *ac = themeMenu->addAction(f.baseName(), this, SLOT(changeTheme()));
        ac->setCheckable(true);
        if (ac->text() == theme)
            ac->setChecked(true);
        ag->addAction(ac);
    }
}

#undef MainEngineRoutine
