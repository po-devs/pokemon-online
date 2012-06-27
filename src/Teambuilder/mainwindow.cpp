#include "mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "menu.h"
#include "client.h"
#include "serverchoice.h"
#include "../PokemonInfo/movesetchecker.h"
#include "pluginmanager.h"
#include "plugininterface.h"
#include "theme.h"
#include "logmanager.h"
#include "replayviewer.h"
#include "../Utilities/functions.h"
#include "Teambuilder/teamholder.h"
#include "Teambuilder/teambuilder.h"

MainEngine::MainEngine() : displayer(0)
{
    m_team = new TeamHolder();

    pluginManager = new PluginManager(this);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QSettings s;
    /* initializing the default init values if not there */
    setDefaultValue(s, "Theme", "Themes/Classic/");
    setDefaultValue(s, "battle_cry_volume", 100);
    setDefaultValue(s, "battle_music_volume", 100);
    setDefaultValue(s, "profiles_path", appDataPath("Profiles", true));
    setDefaultValue(s, "current_profile", appDataPath("Profiles", false));

#ifdef Q_OS_MACX
    setDefaultValue(s, "team_folder", QDir::homePath() + "/Documents");
    setDefaultValue(s, "team_location", QDir::homePath() + "/Documents/trainer.tp");
    setDefaultValue(s, "user_theme_directory", QDir::homePath() + "/Documents/Pokemon Online Themes/");
#else
    setDefaultValue(s, "team_location", "Team/trainer.tp");
    setDefaultValue(s, "team_folder", "Team");
    setDefaultValue(s, "user_theme_directory", "Themes/");
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
    setDefaultValue(s, "reject_incoming_pms", false);
    setDefaultValue(s, "pms_tabbed", false);
    setDefaultValue(s, "pms_logged", true);
    setDefaultValue(s, "animate_hp_bar", true);
    setDefaultValue(s, "sort_players_by_tier", false);
    setDefaultValue(s, "sort_channels_by_name", false);
    setDefaultValue(s, "show_all_items", false);
    setDefaultValue(s, "animated_sprites", false);

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
    GenInfo::init("db/gens/");
    Theme::init(s.value("Theme").toString());

    /* Loading the values */
    QApplication::setStyle("plastique");
    loadStyleSheet();

    trainerTeam()->load();

    launchMenu(true);
}

MainEngine::~MainEngine()
{
    delete pluginManager, pluginManager = NULL;
    delete m_team, m_team = NULL;
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

ThemeAccessor *MainEngine::theme()
{
    return Theme::getAccessor();
}

void MainEngine::loadStyleSheet()
{
    QFile stylesheet(Theme::path("default.css"));
    stylesheet.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(stylesheet.readAll());
}

void MainEngine::routine(CentralWidgetInterface *w)
{
    displayer->setWindowTitle(tr("Pokemon Online"));
    central->addWidget(dynamic_cast<QWidget*>(w));
    QWidget *toDel = central->widget(0);
    central->removeWidget(central->widget(0));
    displayer->setMenuBar(transformMenuBar(w->createMenuBar(this)));
    //loadSettings(dynamic_cast<QWidget*>(w), w->defaultSize());

    toDel->deleteLater();
}

void MainEngine::launchMenu(bool first)
{
    Menu *menu = new Menu();
    if (first) {
        displayer = new QMainWindow();
        displayer->resize(menu->size());
        displayer->setWindowTitle(tr("Pokemon Online"));
        displayer->setCentralWidget(central = new QStackedWidget);
        central->setObjectName("CentralWidget");
        central->addWidget(menu);
        displayer->setMenuBar(transformMenuBar(menu->createMenuBar(this)));
        loadSettings(menu, menu->defaultSize());\
        displayer->show();
    } else {
        routine(menu);
    }

    connect(menu, SIGNAL(goToTeambuilder()), SLOT(launchTeamBuilder()));
    connect(menu, SIGNAL(goToExit()), SLOT(quit()));
    connect(menu, SIGNAL(goToOnline()), SLOT(launchServerChoice()));
    connect(menu, SIGNAL(goToCredits()), SLOT(launchCredits()));
}

void MainEngine::launchCredits()
{
    QFile fichier("db/credits.html");
    if(!fichier.open(QIODevice::ReadOnly)) {
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
    //routine(d_credit);
    d_credit.setLayout(l);
    d_credit.move(this->displayer->geometry().x(),this->displayer->geometry().y());
    d_credit.setStyleSheet(
                "QLabel {background:transparent}"
                           );
    d_credit.exec();
}

void MainEngine::launchTeamBuilder()
{
    TeamBuilder *TB = new TeamBuilder(trainerTeam());
    routine(TB);

    connect(TB, SIGNAL(done()), SLOT(launchMenu()));
    connect(TB, SIGNAL(reloadMenuBar()), SLOT(updateMenuBar()));
}

void MainEngine::launchServerChoice()
{
    ServerChoice *choice = new ServerChoice(trainerTeam()->name());
    routine(choice);

    connect(choice, SIGNAL(rejected()), SLOT(launchMenu()));
    connect(choice, SIGNAL(serverChosen(QString,quint16,QString)), this, SLOT(goOnline(QString,quint16,QString)));
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

    QString fullTheme = Theme::FindTheme(theme);
    qDebug() << fullTheme;
    if (!fullTheme.isNull()) {
        settings.setValue("Theme", fullTheme);

        Theme::Reload(fullTheme);
        loadStyleSheet();
    }
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
    if (nick.size() > 0) {
        trainerTeam()->name() = nick;
    }

    if (trainerTeam()->name().length() == 0) {
        QMessageBox::information(displayer, tr("Impossible to go online"), tr("You haven't set your name yet. Do so in the teambuilder."));
        return;
    }

    Client * client = new Client(pluginManager, trainerTeam(), url, port);
    routine(client);

    connect(client, SIGNAL(done()), SLOT(launchMenu()));
}

void MainEngine::updateMenuBar()
{
    displayer->setMenuBar(transformMenuBar(dynamic_cast<CentralWidgetInterface*>(central->currentWidget())
                            ->createMenuBar(this)));
}

void MainEngine::quit()
{
    /* Has to be deleted here, otherwise windows error if the libraries are not detached */
    delete pluginManager, pluginManager = NULL;
    displayer->close();
}

void MainEngine::loadTeamDialog()
{
    loadTTeamDialog(trainerTeam()->team());
}

void MainEngine::loadReplayDialog()
{
    QFileDialog *f = new QFileDialog(NULL, QObject::tr("Replay a battle"), LogManager::obj()->getDirectoryForType(ReplayLog), "*.poreplay");
    f->setWindowFlags(Qt::Window);
    f->setAttribute(Qt::WA_DeleteOnClose);
    f->setAcceptMode(QFileDialog::AcceptOpen);
    f->setFileMode(QFileDialog::ExistingFile);
    f->show();

    connect(f, SIGNAL(fileSelected(QString)), SLOT(showReplay(QString)));
}

void MainEngine::showReplay(QString file)
{
    new ReplayViewer(file);
}

void MainEngine::addThemeMenu(QMenuBar *menuBar)
{
    themeMenu = menuBar->addMenu(tr("&Theme"));
    rebuildThemeMenu();
}

void MainEngine::rebuildThemeMenu()
{
    themeMenu->clear();

    themeMenu->addAction(tr("Change &user theme folder ..."), this, SLOT(changeUserThemeFolder()));
    themeMenu->addSeparator();

    QSettings s;

    QStringList searchPath = Theme::SearchPath();

    QSet<QString> themes;
    foreach(QString dir, searchPath) {
        QDir d(dir);
        foreach(QFileInfo f, d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            themes.insert(f.baseName());
        }
    }

    QString theme = s.value("Theme").toString().section('/', -2, -2);

    QActionGroup *ag = new QActionGroup(themeMenu);
    foreach(QString baseName, themes) {
        QAction *ac = themeMenu->addAction(baseName, this, SLOT(changeTheme()));
        ac->setCheckable(true);
        if (ac->text() == theme)
            ac->setChecked(true);
        ag->addAction(ac);
    }

    themeMenu->addSeparator();
    themeMenu->addAction(tr("Reload &StyleSheet"), this, SLOT(loadStyleSheet()));
}

void MainEngine::changeUserThemeFolder()
{
    QSettings s;
    QString dir = QFileDialog::getExistingDirectory(displayer, tr("User Theme Directory"), s.value("user_theme_directory").toString());

    if (dir != "") {
        s.setValue("user_theme_directory", dir + "/");
    }
    rebuildThemeMenu();
}


#undef routine
