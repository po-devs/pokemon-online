#include "menu.h"
#include "ui_menu.h"
#include "../Utilities/functions.h"
#include "mainwindow.h"
#include "theme.h"

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

    loadSettings(this);
}

Menu::~Menu()
{
    /* We only save size settings on the first main screen window. We don't save the size if the user went online and maximized their
      online window for example.

      An improvement of this could be saving the size only if the user resized the window during the widget's lifetime
 */
    static bool menuLoaded = false;

    if (!menuLoaded) {
        writeSettings(this);
        menuLoaded = true;
    }

    delete ui;
}

QMenuBar * Menu::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&Load Team"),w,SLOT(loadTeamDialog()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("Open &replay"),w,SLOT(loadReplayDialog()), Qt::CTRL+Qt::Key_R);
    menuFichier->addAction(tr("&Quit"),qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);

    w->addThemeMenu(menuBar);

    QMenu *langMenu = menuBar->addMenu(tr("&Language"));
    QFile in ("languages.txt");
    in.open(QIODevice::ReadOnly);
    QStringList langs = QString::fromUtf8(in.readAll()).split('\n');
    foreach(QString a, langs) {
        langMenu->addAction(a,w, SLOT(changeLanguage()));
    }

    return menuBar;
}
