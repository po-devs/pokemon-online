#include "menu.h"
#include "ui_menu.h"
#include "../Utilities/functions.h"
#include "mainwindow.h"
#include "theme.h"
#include "Teambuilder/teamholder.h"
#include "loadwindow.h"

static bool menuLoaded = false;

Menu::Menu(TeamHolder *t) :
    ui(new Ui::Menu), team(t)
{
    ui->setupUi(this);
    ui->appLogo->setPixmap(Theme::Sprite("logo"));
    ui->updateContainer->hide();
    ui->stackedWidget->setCurrentIndex(1);

    setWindowTitle(tr("Menu"));

    connect (ui->teamBuilder, SIGNAL(clicked()), SIGNAL(goToTeambuilder()));
    connect (ui->goOnline, SIGNAL(clicked()), SIGNAL(goToOnline()));
    connect (ui->credits, SIGNAL(clicked()), SIGNAL(goToCredits()));
    connect (ui->exit, SIGNAL(clicked()), SIGNAL(goToExit()));
    connect (ui->updateButton, SIGNAL(clicked()), SIGNAL(downloadUpdateRequested()));
    connect (ui->prevTip, SIGNAL(clicked()), SLOT(prevTip()));
    connect (ui->nextTip, SIGNAL(clicked()), SLOT(nextTip()));

    srand(time(NULL));

    msgs = QString::fromUtf8(getFileContent("db/tips.txt")).trimmed().split("\n");

    currentTip = rand()%(msgs.size());
    updateTip();

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

void Menu::updateTip()
{
    ui->tip->setHtml(msgs.at(currentTip));
    ui->tipNumber->setText(tr("%1 / %2").arg(currentTip+1).arg(msgs.size()));
}

void Menu::setUpdateData(const QString &data)
{
    ui->updateLabel->setText(data);
    ui->changeLog->setText(tr("Loading changelog..."));
    ui->updateContainer->show();
    ui->stackedWidget->setCurrentIndex(0);
}

void Menu::disableUpdateButton()
{
    ui->updateButton->setDisabled(true);
}

void Menu::showChangeLog()
{
    ui->updateButton->hide();
    ui->updateContainer->show();
    ui->updateLabel->setText(tr("Pokemon Online was just updated!"));

    QFile in("changelog.txt");
    in.open(QIODevice::ReadOnly);
    ui->changeLog->setText(QString::fromUtf8(in.readAll()));
}

void Menu::setChangeLogData(const QString &data)
{
    ui->changeLog->setText(data);
}

void Menu::on_updateButton_clicked()
{
    ui->updateButton->setText(tr("Downloading update..."));
    ui->updateButton->setEnabled(false);
}

bool Menu::event(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        e->accept();
    }

    return QFrame::event(e);
}

void Menu::loadTeam()
{
    LoadWindow *w = new LoadWindow(this, QStringList(), team->name());
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    w->show();

    connect(w, SIGNAL(teamLoaded(TeamHolder)), SLOT(loadAll(TeamHolder)));
}

void Menu::loadAll(const TeamHolder& t)
{
    *team = t;
}

QMenuBar * Menu::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("Load team"), this, SLOT(loadTeam()), tr("Ctrl+L", "Load team"));
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
void Menu::prevTip()
{
    currentTip = (currentTip - 1 + msgs.size()) % msgs.size();
    updateTip();
}

void Menu::nextTip()
{
    currentTip = (currentTip + 1) % msgs.size();
    updateTip();
}
