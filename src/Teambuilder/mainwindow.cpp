#ifdef QT5
#include <QApplication>
#include <QDialog>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QStandardPaths>
#include <QStyleFactory>
#endif
#include <QtCore/QVariant>

#include <Utilities/functions.h>
#include <Utilities/pluginmanagerdialog.h>
#include <PokemonInfo/teamholder.h>
#include <PokemonInfo/pokemoninfo.h>
#include <PokemonInfo/movesetchecker.h>

#include <TeambuilderLibrary/theme.h>
#include "Teambuilder/teambuilder.h"

#include "mainwindow.h"
#include "menu.h"
#include "client.h"
#include "serverchoice.h"
#include "pluginmanager.h"
#include "plugininterface.h"
#include "logmanager.h"
#include "replayviewer.h"
#include "mainwidget.h"
#include "downloadmanager.h"

#ifdef Q_OS_MACX
#include "mac/FullScreenSupport.h"
#endif

MainEngine *MainEngine::inst = NULL;

static void setDefaultValues()
{
    QSettings s;
    /* initializing the default init values if not there */
    setDefaultValue(s, "Themes/Current", "Themes/Classic/");
    setDefaultValue(s, "BattleAudio/CryVolume", 100);
    setDefaultValue(s, "BattleAudio/MusicVolume", 100);
    setDefaultValue(s, "BattleAudio/MusicDirectory", "Music/Battle/");
    setDefaultValue(s, "BattleAudio/PlayMusic", false);
    setDefaultValue(s, "BattleAudio/PlaySounds", false);
    setDefaultValue(s, "Profile/Path", appDataPath("Profiles", true));
    setDefaultValue(s, "Profile/Current", appDataPath("Profiles", false));

#ifdef QT5
    const QString docLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
    const QString docLocation = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
#ifdef Q_OS_MACX
    setDefaultValue(s, "Teams/Folder", docLocation + "/Teams/");
    setDefaultValue(s, "Themes/Directory", docLocation + "/Pokemon Online Themes/");
#else
    setDefaultValue(s, "Teams/Folder", docLocation + "/Pokemon Online/Teams/");
    setDefaultValue(s, "Themes/Directory", "Themes/");
#endif
    /* Creates the team folder by default so users don't get an error when saving a team for the
     *first time */
    QString teamPath = s.value("Teams/Folder").toString();
    QDir d;
    d.mkpath(teamPath);

    setDefaultValue(s, "Battle/FlashOnMove", true);
    setDefaultValue(s, "Battle/AnimateHp", true);
    setDefaultValue(s, "Battle/OldWindow", true);
    setDefaultValue(s, "Battle/OldAttackButtons", true);
    setDefaultValue(s, "Client/EnableLadder", true);
    setDefaultValue(s, "Client/SortPlayersByTier", false);
    setDefaultValue(s, "Client/SortChannelsByName", true);
    setDefaultValue(s, "Client/ShowTimestamps", true);
    setDefaultValue(s, "PlayerEvents/ShowIdle", false);
    setDefaultValue(s, "PlayerEvents/ShowBattle", false);
    setDefaultValue(s, "PlayerEvents/ShowChannel", false);
    setDefaultValue(s, "PlayerEvents/ShowTeam", false);
    setDefaultValue(s, "PMs/ShowTimestamps", true);
    setDefaultValue(s, "PMs/ShowSeconds", false);
    //setDefaultValue(s, "PMs/Flash", true); //deleted that feature
    setDefaultValue(s, "PMs/RejectIncoming", false);
    setDefaultValue(s, "PMs/Tabbed", true);
    setDefaultValue(s, "PMs/Logged", true);
    setDefaultValue(s, "PMs/Notifications", true);
    setDefaultValue(s, "Mods/CurrentMod", QString());
    setDefaultValue(s, "TeamBuilder/ShowAllItems", false);
    setDefaultValue(s, "animated_sprites", false);
}

MainEngine::MainEngine(bool updated) : displayer(0), freespot(0)
{
    inst = this;

    setProperty("updated", updated);

    pluginManager = new ClientPluginManager(this);

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

    setDefaultValues();

    QSettings s;

    pmNotify = s.value("PMs/Notifications").isNull() ? true : s.value("PMs/Notifications").toBool();

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

    QString locale = s.value("language").toString();

    PokemonInfoConfig::setFillMode(FillMode::Client);
    PokemonInfoConfig::changeMod(s.value("Mods/CurrentMod").toString());
    PokemonInfoConfig::changeTranslation(locale);

    reloadPokemonDatabase();

    Theme::init(s.value("Themes/Current").toString());

    /* Loading the values */
#ifdef QT5
    if (s.value("application_style").toString().toLower() == "plastique") {
        s.remove("application_style");
    }
    QApplication::setStyle(s.value("application_style", "Fusion").toString());
#else
    QApplication::setStyle(s.value("application_style", "plastique").toString());
#endif
    loadStyleSheet();

    connect(&downloader, SIGNAL(updatesAvailable(QString,bool)), SLOT(updateDataReady(QString,bool)));
    connect(&downloader, SIGNAL(changeLogAvailable(QString,bool)), SLOT(changeLogReady(QString,bool)));
#ifndef Q_OS_MACX
    // On Mac OSX, we do not want two update checkers
    // Hence, we disable the default one, as updating Mac OSX
    // application bundles is different from Linux/Windows
    downloader.loadUpdatesAvailable();
#endif

    //launchMenu(true);
    displayer = new QMainWindow();
#ifdef Q_OS_MACX
    MacSupport::setupFullScreen(displayer);
#endif

    displayer->setCentralWidget(main = new MainWidget());
    connect(main, SIGNAL(reloadMenuBar()), SLOT(updateMenuBar()));
    displayer->show();

    launchServerChoice();

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), SLOT(updateRunningTime()));

    /* This is to detect other PO running at the same time */
    updateRunningTime();
    t->start(60*1000);

    /* Tray icon */
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("Pok\303\251mon Online Client");
    trayIcon->setIcon(QIcon("db/icon.png"));
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), displayer, SLOT(raise()));
    connect(trayIcon, SIGNAL(messageClicked()), SLOT(raiseLastNotificationSender()));
}

MainEngine::~MainEngine()
{
    delete pluginManager, pluginManager = NULL;

    clearTrash();

    foreach(TeamHolder *h, m_teams) {
        delete h;
    }
    m_teams.clear();
}

void MainEngine::updateDataReady(const QString &data, bool error)
{
    updateData = data;

    if (!error) {
        downloader.loadChangelog();
    }
}

void MainEngine::changeLogReady(const QString &data, bool error)
{
    (void) error;
    changeLog = data;
}

TeamHolder* MainEngine::trainerTeam(int spot)
{
    if (!m_teams.contains(spot)) {
        m_teams[spot] = new TeamHolder();
        m_teams[spot]->load();
    }
    return m_teams.value(spot);
}

TeamHolder *MainEngine::trainerTeam()
{
    return trainerTeam(currentSpot());
}

int MainEngine::currentSpot() const
{
    return main->currentWidget() ? main->currentWidget()->property("tab-window").toInt() : freespot;
}

void MainEngine::reloadPokemonDatabase()
{
    QSettings s;

    GenInfo::init("db/gens/");
    PokemonInfo::init("db/pokes/");
    MoveSetChecker::init("db/pokes/", s.value("TeamBuilder/EnforceMinLevels").toBool());
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    GenderInfo::init("db/genders/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");
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
    PluginManagerDialog *d = new PluginManagerDialog(displayer->centralWidget());
    d->setWindowFlags(Qt::Window);
    d->setPluginManager(pluginManager);
    d->show();

    connect(d, SIGNAL(accepted()), SLOT(updateMenuBar()));
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

void MainEngine::openThemesForum()
{
    QDesktopServices::openUrl(QUrl("http://pokemon-online.eu/forums/themes.92/"));
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

void MainEngine::clearTrash()
{
    foreach(TeamHolder* t, trash) {
        delete t;
    }
    trash.clear();
}

void MainEngine::routine(CentralWidgetInterface *w)
{
    clearTrash();

    displayer->setWindowTitle(tr("Pokemon Online"));
    QWidget *wi = dynamic_cast<QWidget*>(w);
    if (wi->property("tab-window").isNull()) {
        wi->setProperty("tab-window", sender() ? sender()->property("tab-window") : freespot);
    }
    if (main->currentWidget() == NULL) {
        //displayer->resize(wi->size());
    }
    main->setWidget(wi->property("tab-window").toInt(), wi);

    displayer->setMenuBar(transformMenuBar(w->createMenuBar(this)));
}

void MainEngine::launchMenu()
{
    Menu *menu = new Menu(trainerTeam());

    routine(menu);

    if (!property("updated").toBool()) {
        if (updateData.length() > 0) {
            menu->setUpdateData(updateData);
        } else {
            connect(&downloader, SIGNAL(updatesAvailable(QString,bool)), menu, SLOT(setUpdateData(QString)));
        }

        if (changeLog.length() > 0) {
            menu->setChangeLogData(changeLog);
        } else {
            connect(&downloader, SIGNAL(changeLogAvailable(QString,bool)), menu, SLOT(setChangeLogData(QString)));
        }

        if (downloader.updateReady() || downloader.isDownloading()) {
            menu->disableUpdateButton();
        }

        connect(menu, SIGNAL(downloadUpdateRequested()), &downloader, SLOT(downloadUpdate()));
    } else {
        menu->showChangeLog();
    }

    connect(menu, SIGNAL(goToTeambuilder()), SLOT(launchTeamBuilder()));
    connect(menu, SIGNAL(goToExit()), SLOT(closeTab()));
    connect(menu, SIGNAL(goToOnline()), SLOT(launchServerChoice()));
    connect(menu, SIGNAL(goToCredits()), SLOT(launchCredits()));
}

void MainEngine::updateRunningTime()
{
    QSettings settings;
    settings.setValue("Updates/RunningTime", QString::number(::time(NULL)));
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
    credit->setOpenExternalLinks(true);
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
    d_credit.setStyleSheet(
                "QLabel {background:transparent}"
                           );
    d_credit.exec();
}

void MainEngine::launchTeamBuilder()
{
    TeamBuilder *TB = new TeamBuilder(pluginManager, trainerTeam(), false);
    routine(TB);

    connect(TB, SIGNAL(done()), SLOT(launchServerChoice()));
    connect(TB, SIGNAL(reloadMenuBar()), SLOT(updateMenuBar()));
    connect(TB, SIGNAL(reloadDb()), SLOT(reloadPokemonDatabase()));
}

void MainEngine::launchServerChoice(bool newTab)
{
    ServerChoice *choice;
    if (newTab) {
        choice = new ServerChoice(trainerTeam(++freespot));
        choice->setProperty("tab-window", freespot);
    } else {
        choice = new ServerChoice(trainerTeam());
    }

    routine(choice);

    if (!this->property("first launch").toBool()) {
        this->setProperty("first launch", true);
        loadSettings(dynamic_cast<QWidget*>(choice), choice->defaultSize());
    }

    connect(choice, SIGNAL(teambuilder()), SLOT(launchTeamBuilder()));
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
        settings.setValue("Themes/Current", fullTheme);

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

    extern QTranslator translator;
    extern QTranslator qtTranslator;

    qtTranslator.load(QString("qt_") + lang,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    translator.load(QString("trans/translation_%1").arg(lang));

    PokemonInfoConfig::changeTranslation(lang);

    PokemonInfo::retranslate();
    MoveInfo::retranslate();
    ItemInfo::retranslate();
    TypeInfo::retranslate();
    NatureInfo::retranslate();
    CategoryInfo::retranslate();
    AbilityInfo::retranslate();
    GenderInfo::retranslate();
    StatInfo::retranslate();
    GenInfo::retranslate();
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

    connect(client, SIGNAL(done()), SLOT(launchServerChoice()));
    connect(client, SIGNAL(titleChanged()), main, SLOT(updateTabNames()));
    connect(client, SIGNAL(pmNotificationsChanged(bool)), SLOT(pmNotificationsChanged(bool)));
}

void MainEngine::updateMenuBar()
{
    displayer->setMenuBar(transformMenuBar(dynamic_cast<CentralWidgetInterface*>(main->currentWidget())
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

void MainEngine::openNewTab()
{
    launchServerChoice(true);
}

int MainEngine::numberOfTabs() const
{
    return main->numberOfTabs();
}

void MainEngine::closeTab()
{
    if (numberOfTabs() <= 1) {
        quit();
    } else {
        int current = currentSpot();
        main->closeTab(current);
        trash.push_back(m_teams.take(current));
    }
}

void MainEngine::pmNotificationsChanged(bool notify)
{
    pmNotify = notify;
}

void MainEngine::showMessage(const QString &title, const QString &msg)
{
    if (!pmNotify) {
        return;
    }
    lastNotificationSender = dynamic_cast<QWidget*>(sender());
    trayIcon->showMessage(title, msg,QSystemTrayIcon::Information);
}

void MainEngine::raiseLastNotificationSender()
{
    /* Second check: for non-tabbed pms, the hidden tabbed pm window sends the signal, so
        instead we don't do anything */
    if (lastNotificationSender && lastNotificationSender->isVisible()) {
        lastNotificationSender->raise();
    }
}

void MainEngine::loadReplayDialog()
{
    QFileDialog *f = new QFileDialog(NULL, QObject::tr("Replay a battle"), LogManager::obj()->getDirectoryForType(ReplayLog), "*.poreplay");
    //f->setWindowFlags(Qt::Window);
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

void MainEngine::addStyleMenu(QMenuBar *menuBar)
{
    QMenu * menuStyle = menuBar->addMenu(tr("&Style"));
    QStringList style = QStyleFactory::keys();
    QActionGroup *ag = new QActionGroup(menuBar);

    QSettings settings;
#ifdef QT5
    QString curStyle = settings.value("application_style", "Fusion").toString();
#else
    QString curStyle = settings.value("application_style", "plastique").toString();
#endif

    foreach(QString s , style) {
        QAction *ac = menuStyle->addAction(s,this,SLOT(changeStyle()));
        ac->setCheckable(true);

        if (s.toLower() == curStyle.toLower()) {
            ac->setChecked(true);
        }
        ag->addAction(ac);
    }
}

void MainEngine::addLanguageMenu(QMenuBar *menuBar)
{
    QMenu *langMenu = menuBar->addMenu(tr("&Language"));
    QFile in ("languages.txt");
    in.open(QIODevice::ReadOnly);

    QSettings s;
    QStringList langs = QString::fromUtf8(in.readAll()).trimmed().split('\n');
    QActionGroup *ag = new QActionGroup(langMenu);
    foreach(QString a, langs) {
        QAction *act = langMenu->addAction(a, this, SLOT(changeLanguage()));
        act->setCheckable(true);
        act->setChecked(s.value("language").toString() == a.section("(", 1).section(")", 0, 0));
        ag->addAction(act);
    }
}

void MainEngine::rebuildThemeMenu()
{
    themeMenu->clear();

    themeMenu->addAction(tr("Change &user theme folder ..."), this, SLOT(changeUserThemeFolder()));
    themeMenu->addSeparator();

    QSettings s;

    QStringList searchPath = Theme::SearchPaths();

    QSet<QString> themes;
    foreach(QString dir, searchPath) {
        QDir d(dir);
        foreach(QFileInfo f, d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            themes.insert(f.baseName());
        }
    }

    QString theme = s.value("Themes/Current").toString().section('/', -2, -2);

    QActionGroup *ag = new QActionGroup(themeMenu);
    foreach(QString baseName, themes) {
        QAction *ac = themeMenu->addAction(baseName, this, SLOT(changeTheme()));
        ac->setCheckable(true);
        if (ac->text() == theme)
            ac->setChecked(true);
        ag->addAction(ac);
    }

    themeMenu->addSeparator();
    themeMenu->addAction(tr("Reload &StyleSheet"), this, SLOT(loadStyleSheet()), tr("Ctrl+D", "Reload Stylesheet"));
    themeMenu->addSeparator();
    themeMenu->addAction(tr("&Get more themes..."), this, SLOT(openThemesForum()));
}

void MainEngine::changeUserThemeFolder()
{
    QSettings s;
    QString dir = QFileDialog::getExistingDirectory(displayer, tr("User Theme Directory"), s.value("Themes/Directory").toString());

    if (dir != "") {
        s.setValue("Themes/Directory", dir + "/");
    }
    rebuildThemeMenu();
}


