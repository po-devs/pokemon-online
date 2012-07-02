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
    setDefaultValue(s, "Themes/Current", "Themes/Classic/");
    setDefaultValue(s, "BattleAudio/CryVolume", 100);
    setDefaultValue(s, "BattleAudio/MusicVolume", 100);
    setDefaultValue(s, "BattleAudio/MusicDirectory", "Music/Battle/");
    setDefaultValue(s, "BattleAudio/PlayMusic", false);
    setDefaultValue(s, "BattleAudio/PlaySounds", false);
    setDefaultValue(s, "Profile/Path", appDataPath("Profiles", true));
    setDefaultValue(s, "Profile/Current", appDataPath("Profiles", false));

#ifdef Q_OS_MACX
    setDefaultValue(s, "Teams/Folder", QDir::homePath() + "/Documents");
    setDefaultValue(s, "Themes/Directory", QDir::homePath() + "/Documents/Pokemon Online Themes/");
#else
    setDefaultValue(s, "Teams/Folder", "Team");
    setDefaultValue(s, "Themes/Directory", "Themes/");
#endif
    setDefaultValue(s, "Battle/FlashOnMove", true);
    setDefaultValue(s, "Battle/AnimateHp", true);
    setDefaultValue(s, "Client/EnableLadder", true);
    setDefaultValue(s, "Client/SortPlayersByTier", false);
    setDefaultValue(s, "Client/SortChannelsByName", true);
    setDefaultValue(s, "Client/ShowTimestamps", true);
    setDefaultValue(s, "PlayerEvents/ShowIdle", false);
    setDefaultValue(s, "PlayerEvents/ShowBattle", false);
    setDefaultValue(s, "PlayerEvents/ShowChannel", false);
    setDefaultValue(s, "PlayerEvents/ShowTeam", false);
    setDefaultValue(s, "PMs/ShowTimestamps", true);
    setDefaultValue(s, "PMs/Flash", true);
    setDefaultValue(s, "PMs/RejectIncoming", false);
    setDefaultValue(s, "PMs/Tabbed", true);
    setDefaultValue(s, "PMs/Logged", true);
    setDefaultValue(s, "Mods/CurrentMod", QString());
    setDefaultValue(s, "TeamBuilder/ShowAllItems", false);
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

    PokemonInfoConfig::changeMod(s.value("Mods/CurrentMod").toString(), FillMode::Client);

    reloadPokemonDatabase();

    Theme::init(s.value("Themes/Current").toString());

    /* Loading the values */
    QApplication::setStyle(s.value("application_style", "plastique").toString());
    loadStyleSheet();

    trainerTeam()->load();

    launchMenu(true);
}

MainEngine::~MainEngine()
{
    delete pluginManager, pluginManager = NULL;
    delete m_team, m_team = NULL;
}

void MainEngine::reloadPokemonDatabase()
{
    QSettings s;

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
    GenInfo::init("db/gens/");
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
    TeamBuilder *TB = new TeamBuilder(trainerTeam(), false);
    routine(TB);

    connect(TB, SIGNAL(done()), SLOT(launchMenu()));
    connect(TB, SIGNAL(reloadMenuBar()), SLOT(updateMenuBar()));
    connect(TB, SIGNAL(reloadDb()), SLOT(reloadPokemonDatabase()));
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
    translator.load(QString("trans/%1/translation_%1").arg(lang));

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

void MainEngine::addStyleMenu(QMenuBar *menuBar)
{
    QMenu * menuStyle = menuBar->addMenu(tr("&Style"));
    QStringList style = QStyleFactory::keys();
    QActionGroup *ag = new QActionGroup(menuBar);

    QSettings settings;
    QString curStyle = settings.value("application_style", "plastique").toString();

    foreach(QString s , style) {
        QAction *ac = menuStyle->addAction(s,this,SLOT(changeStyle()));
        ac->setCheckable(true);

        if (s.toLower() == curStyle.toLower()) {
            ac->setChecked(true);
        }
        ag->addAction(ac);
    }
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
    themeMenu->addAction(tr("Reload &StyleSheet"), this, SLOT(loadStyleSheet()));
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


#undef routine
