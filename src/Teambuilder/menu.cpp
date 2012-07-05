#include "menu.h"
#include "ui_menu.h"
#include "../Utilities/functions.h"
#include "mainwindow.h"
#include "theme.h"

static bool menuLoaded = false;

Menu::Menu(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);
    ui->appLogo->setPixmap(Theme::Sprite("logo"));

    setWindowTitle(tr("Menu"));

    connect (ui->teamBuilder, SIGNAL(clicked()), SIGNAL(goToTeambuilder()));
    connect (ui->goOnline, SIGNAL(clicked()), SIGNAL(goToOnline()));
    connect (ui->credits, SIGNAL(clicked()), SIGNAL(goToCredits()));
    connect (ui->exit, SIGNAL(clicked()), SIGNAL(goToExit()));

    if (!menuLoaded) {
        loadSettings(this);
    }
}

Menu::~Menu()
{
    /* We only save size settings on the first main screen window. We don't save the size if the user went online and maximized their
      online window for example.

      An improvement of this could be saving the size only if the user resized the window during the widget's lifetime
 */
    if (!menuLoaded) {
        writeSettings(this);
        menuLoaded = true;
    }

    delete ui;
}

bool Menu::event(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        e->accept();
    }

    return QFrame::event(e);
}

QMenuBar * Menu::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("Close tab"), w, SLOT(closeTab()), tr("Ctrl+W", "Close tab"));
    fileMenu->addAction(tr("Open &replay"),w,SLOT(loadReplayDialog()), Qt::CTRL+Qt::Key_R);
    fileMenu->addAction(tr("&Quit"),qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);

    w->addThemeMenu(menuBar);
    w->addStyleMenu(menuBar);

    QMenu *langMenu = menuBar->addMenu(tr("&Language"));
    QFile in ("languages.txt");
    in.open(QIODevice::ReadOnly);

    QSettings s;
    QStringList langs = QString::fromUtf8(in.readAll()).trimmed().split('\n');
    QActionGroup *ag = new QActionGroup(langMenu);
    foreach(QString a, langs) {
        QAction *act = langMenu->addAction(a,w, SLOT(changeLanguage()));
        act->setCheckable(true);
        act->setChecked(s.value("language").toString() == a.section("(", 1).section(")", 0, 0));
        ag->addAction(act);
    }

    return menuBar;
}
