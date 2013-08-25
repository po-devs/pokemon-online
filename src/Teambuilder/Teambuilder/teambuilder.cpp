#include "../PokemonInfo/pokemon.h"
#include "../PokemonInfo/geninfo.h"
#include "../Utilities/functions.h"
#include "Teambuilder/teambuilder.h"
#include "Teambuilder/trainermenu.h"
#include "Teambuilder/teamholder.h"
#include "mainwindow.h"
#include "Teambuilder/teammenu.h"
#include "Teambuilder/pokeboxes.h"
#include "Teambuilder/poketablemodel.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "loadwindow.h"
#include "pluginmanager.h"

#ifdef _WIN32
#include "../../SpecialIncludes/zip.h"
#else
#include <zip.h>
#endif

#include <cerrno>

TeamBuilder::TeamBuilder(PluginManager *p, TeamHolder *team, bool load) : m_team(team), teamMenu(NULL)
{
    ui = new _ui();
    ui->stack = new QStackedWidget();
    setCentralWidget(ui->stack);

    setDockNestingEnabled(true);
    setWindowFlags(Qt::Widget);

    setWindowTitle(tr("Teambuilder"));

    ui->stack->addWidget(trainer = new TrainerMenu(team));
    trainer->setMainWindow(this);
    pokemonModel = new PokeTableModel(team->team().gen(), this);

    if (load) {
        loadSettings(this, defaultSize());
    }

    connect(trainer, SIGNAL(teamChanged()), SLOT(markTeamUpdated()));
    connect(trainer, SIGNAL(done()), SIGNAL(done()));
    connect(trainer, SIGNAL(openBoxes()), SLOT(openBoxes()));
    connect(trainer, SIGNAL(editPoke(int)), SLOT(editPoke(int)));

    p->launchTeambuilder(this);
    pluginManager = p;
}

TeamBuilder::~TeamBuilder()
{
    pluginManager->quitTeambuilder(this);
    writeSettings(this);
    delete ui;
}

QSize TeamBuilder::defaultSize() const {
    return QSize(600,400);
}

QMenuBar *TeamBuilder::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    menuBar->setObjectName("TeamBuilder");
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New"),this,SLOT(newTeam()),tr("Ctrl+N", "New"));
    fileMenu->addAction(tr("&Save all"),this,SLOT(saveAll()),tr("Ctrl+S", "Save all"));
    fileMenu->addAction(tr("&Load all"),this,SLOT(openLoadWindow()),tr("Ctrl+L", "Load all"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"),qApp,SLOT(quit()),tr("Ctrl+Q", "Quit"));
    QMenu *teamMenu = menuBar->addMenu(tr("&Team"));
    if (currentWidget() && currentWidget() == this->teamMenu) {
        teamMenu->addAction(tr("Choose pokemon"), this->teamMenu, SLOT(choosePokemon()), tr("Alt+E", "Choose Pokemon"));
    }
    teamMenu->addAction(tr("Trainer Menu"), this, SLOT(switchToTrainer()), tr("Ctrl+B", "Trainer Menu"));
    teamMenu->addSeparator();
    teamMenu->addAction(tr("&Add team"), this, SLOT(addTeam()), tr("Ctrl+A", "Add team"));
    teamMenu->addAction(tr("&Load team"), this, SLOT(openTeam()), tr("Ctrl+Shift+L", "Load team"));
    teamMenu->addAction(tr("&Save team"), this, SLOT(saveTeam()), tr("Ctrl+Shift+S", "Save team"));
    teamMenu->addAction(tr("&Import team"), this, SLOT(importTeam()), tr("Ctrl+I", "Import team"));
    teamMenu->addAction(tr("&Export team"), this, SLOT(exportTeam()), tr("Ctrl+E", "Export team"));

    currentWidget()->addMenus(menuBar);

    /* Loading mod menu */
    QMenu *menuMods = menuBar->addMenu(tr("&Mods"));
    QActionGroup *group = new QActionGroup(menuMods);

    QString currentMod = PokemonInfoConfig::currentMod();
    // No mod option.
    QAction *noMod = menuMods->addAction(tr("&No mod"), this, SLOT(setNoMod()));
    noMod->setCheckable(true);
    noMod->setChecked(currentMod.length()==0);
    group->addAction(noMod);

    menuMods->addSeparator();

    QStringList mods = PokemonInfoConfig::availableMods();

    foreach(QString smod, mods) {
        QAction *mod = menuMods->addAction(smod, this, SLOT(changeMod()));
        mod->setProperty("name", smod);
        mod->setCheckable(true);
        mod->setChecked(currentMod == smod);
        group->addAction(mod);
    }

    menuMods->addSeparator();
    menuMods->addAction(tr("&Install new mod..."), this, SLOT(installMod()));
    menuMods->addAction(tr("&Remove mod..."), this, SLOT(removeMods()));

    w->addThemeMenu(menuBar);
    w->addStyleMenu(menuBar);

    QMenu *gen = menuBar->addMenu(tr("&Gen."));
    QActionGroup *gens = new QActionGroup(gen);

    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++) {
        int n = GenInfo::NumberOfSubgens(i);

        gen->addSeparator()->setText(GenInfo::Gen(i));

        for (int j = 0; j < n; j++) {
            Pokemon::gen g(i, j);

            ui->gens[g] = gen->addAction(GenInfo::Version(g), this, SLOT(genChanged()));
            ui->gens[g]->setCheckable(true);
            ui->gens[g]->setProperty("gen", QVariant::fromValue(g));
            ui->gens[g]->setChecked(g == team().team().gen());
            gens->addAction(ui->gens[g]);
        }
    }

    lastGen = team().team().gen();

    return menuBar;
}

void TeamBuilder::addPlugin(TeambuilderPlugin *o)
{
    plugins.insert(o);
    hooks.insert(o, o->getHooks());
}

void TeamBuilder::removePlugin(TeambuilderPlugin *o)
{
    plugins.remove(o);
    hooks.remove(o);
}

void TeamBuilder::openLoadWindow()
{
    LoadWindow *w = new LoadWindow(this);
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    w->show();

    connect(w, SIGNAL(teamLoaded(TeamHolder)), SLOT(loadAll(TeamHolder)));
}

void TeamBuilder::genChanged()
{
    Pokemon::gen gen = sender()->property("gen").value<Pokemon::gen>();

    if (gen == team().team().gen()) {
        return;
    }

    team().team().setGen(gen);

    for (int i = 0; i < 6; i++) {
        team().team().poke(i).load();
        team().team().poke(i).runCheck();
    }

    markAllUpdated();
    currentWidget()->updateAll();
}

void TeamBuilder::saveAll()
{
    team().save();
}

void TeamBuilder::loadAll(const TeamHolder &t)
{
    switchToTrainer();
    team() = t;
    markAllUpdated();
    currentWidget()->updateAll();
}

void TeamBuilder::setNoMod()
{
    PokemonInfoConfig::changeMod(QString());

    QSettings settings;
    settings.setValue("Mods/CurrentMod", QString());

    emit reloadDb();
    emit reloadMenuBar();

    markTeamUpdated();
    currentWidget()->updateTeam();
}

void TeamBuilder::changeMod()
{
    QString mod = sender()->property("name").toString();
    PokemonInfoConfig::changeMod(mod);

    QSettings settings;
    settings.setValue("Mods/CurrentMod", mod);

    emit reloadDb();

    markTeamUpdated();
    currentWidget()->updateTeam();
}

void TeamBuilder::installMod()
{
    /* Todo: thread this, and print updated status ? */
#ifdef QT5
    const QString homeLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#else
    const QString homeLocation = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif
    QString archivePath = QFileDialog::getOpenFileName(this, tr("Install mod file"), homeLocation, tr("archive (*.zip)"));

    if (archivePath.isNull()) {
        return;
    }

    QString modName = QFileInfo(archivePath).baseName();

    /****************************/
    /* Extracting the zip... :) */
    /****************************/

    int error = 0;
    char buffer[4096];
    int readsize = 0;

    zip * archive = zip_open(archivePath.toStdString().c_str(), 0, &error);

    if (!archive)
    {
        zip_error_to_str(buffer, 4096, error, errno);
        QMessageBox::critical(this, tr("Impossible to open the archive"), tr("Pokemon Online failed to open the file %1 as an archive (%2).").arg(archivePath, buffer));
        return;
    }

    zip_file *file = zip_fopen(archive, "mod.ini", 0);

    if (!file)
    {
        QMessageBox::critical(this, tr("Incomplete archive"), tr("The file mod.ini couldn't be opened at the base of the archive (%1).").arg(zip_strerror(archive)));
        zip_close(archive);
        return;
    }

    QDir modDir(appDataPath("Mods", true));

    //First remove the mod file if existing
    if (modDir.exists(modName)) {
        qDebug() << "Removing old mod with same name.";
        removeFolder(modDir.absoluteFilePath(modName));
    }

    modDir.mkdir(modName);
    modDir.cd(modName);

    QFile out(modDir.absoluteFilePath("mod.ini"));
    out.open(QIODevice::WriteOnly);

    do
    {
        out.write(buffer, readsize);

        readsize = zip_fread(file, buffer, 4096);
    } while (readsize > 0) ;

    out.close();

    zip_fclose(file), file = NULL;

    /* Now reads all other files */
    //int numFiles = zip_get_num_entries(archive, 0); // <-- when everyone has libzip2, use that
    int numFiles = zip_get_num_files(archive);

    qDebug() << "Number of files in the archive: " << numFiles;

    for (int i = 0; i < numFiles; i++) {
        QString name = zip_get_name(archive, i, 0);

        qDebug() << "File " << i << ": " << name;

        /* Mod.ini is already open */
        if (name != "mod.ini") {
            file = zip_fopen_index(archive, i, 0);

            if (!file) {
                qDebug() << "Opening failed for some reason";
                //error
                continue;
            }

            modDir.mkpath(QFileInfo(name).path());
            QFile out(modDir.absoluteFilePath(name));
            out.open(QIODevice::WriteOnly);

            readsize = 0;

            do
            {
                out.write(buffer, readsize);

                readsize = zip_fread(file, buffer, 4096);
            } while (readsize > 0) ;

            out.close();

            zip_fclose(file), file = NULL;
        }
    }

    zip_close(archive);

    //Done
    emit reloadMenuBar();

    /* If the **** user overwrote his current mod */
    if (modName == PokemonInfoConfig::currentMod()) {
        emit reloadDb();
    }
}

void TeamBuilder::removeMods()
{
    QWidget *widget = new QWidget();
    widget->setWindowTitle(tr("Available mods"));

    QVBoxLayout *v = new QVBoxLayout(widget);

    v->addWidget(modsList = new QListWidget());

    QStringList mods = PokemonInfoConfig::availableMods();

    modsList->addItems(mods);

    QHBoxLayout *buttons = new QHBoxLayout();

    v->addLayout(buttons);

    QPushButton *remove;

    buttons->addWidget(remove = new QPushButton(tr("Remove mod")));

    connect(remove, SIGNAL(clicked()), SLOT(removeMod()));

    widget->show();
}

void TeamBuilder::removeMod()
{
    if (modsList->count() < 1 || modsList->selectedItems().count() < 1) {
        return;
    }

    QString selected = modsList->currentItem()->text();

    if (!selected.isEmpty()) {
        QDir modsDir(appDataPath("Mods", true));

        if (modsDir.exists()) {
            if (modsDir.exists(selected)) {
                removeFolder(modsDir.absoluteFilePath(selected));
                modsList->takeItem(modsList->currentIndex().row());
                reloadMenuBar();
            }
        }
    }
}

void TeamBuilder::newTeam()
{
    switchToTrainer();
    team() = TeamHolder();
    markTeamUpdated();
    currentWidget()->updateAll();
}

void TeamBuilder::addTeam()
{
    if (team().count() >= 6) {
        return;
    }

    team().addTeam();

    if (trainer) {
        trainer->updateAll();
    }

    switchToTrainer();
}

void TeamBuilder::openTeam()
{
    loadTTeamDialog(team().team(), this, SLOT(updateCurrentTeamAndNotify()));
}

void TeamBuilder::updateCurrentTeamAndNotify()
{
    markTeamUpdated();
    currentWidget()->updateTeam();
}

void TeamBuilder::saveTeam()
{
    saveTTeamDialog(team().team(), this, SLOT(onSaveTeam()));
}

void TeamBuilder::onSaveTeam()
{
    if (trainer) {
        trainer->updateTeam();
    }
}

void TeamBuilder::importTeam()
{
    switchToTrainer();
    trainer->openImportDialog();
}

void TeamBuilder::exportTeam()
{
    QTextEdit *exporting = new QTextEdit(this);
    exporting->setObjectName("exporting");
    exporting->setWindowFlags(Qt::Window);
    exporting->setAttribute(Qt::WA_DeleteOnClose, true);

    exporting->setText(team().team().exportToTxt());
    exporting->setReadOnly(true);

    exporting->show();
    exporting->resize(500,500);
}

void TeamBuilder::markAllUpdated()
{
    for (int i = 0; i < ui->stack->count(); i++) {
        if (i != ui->stack->currentIndex()) {
            widget(i)->setProperty("all-to-update", true);
        }
    }
}

void TeamBuilder::markTeamUpdated()
{
    for (int i = 0; i < ui->stack->count(); i++) {
        if (i != ui->stack->currentIndex()) {
            widget(i)->setProperty("team-to-update", true);
        }
    }

    if (team().team().gen() != lastGen) {
        if (ui->gens.contains(team().team().gen())) {
            lastGen = team().team().gen();
            ui->gens[lastGen]->setChecked(true);
        }
    }
}

void TeamBuilder::openBoxes()
{
    editPoke(6);
}

void TeamBuilder::editPoke(int index)
{
    if (!teamMenu) {
        ui->stack->addWidget(teamMenu = new TeamMenu(this, pokemonModel, &team(), index));
        connect(teamMenu, SIGNAL(teamChanged()), SLOT(markTeamUpdated()));
        connect(teamMenu, SIGNAL(switchToTrainer()), SLOT(switchToTrainer()));
    }

    switchTo(teamMenu);

    teamMenu->switchToTab(index);
}

void TeamBuilder::switchToTrainer()
{
    switchTo(trainer);
}

TeamBuilderWidget *TeamBuilder::currentWidget()
{
    return (TeamBuilderWidget*)(ui->stack->currentWidget());
}

TeamBuilderWidget *TeamBuilder::widget(int i)
{
    return (TeamBuilderWidget*)(ui->stack->widget(i));
}

void TeamBuilder::switchTo(TeamBuilderWidget *w)
{
    if (w->property("all-to-update").toBool()) {
        w->updateAll();
        w->setProperty("all-to-update", false);
        w->setProperty("team-to-update", false);
    } else if (w->property("team-to-update").toBool()) {
        w->updateTeam();
        w->setProperty("team-to-update", false);
    }
    currentWidget()->hideAll(); // hide docks used by previous current widget
    ui->stack->setCurrentWidget(w);
    w->showAll(); // show docks used by new current widget

    /* Reloads menu bar, in case the new widget has menus to add */
    emit reloadMenuBar();
}

//TODO
void TeamBuilder::setTierList(const QStringList &tiers)
{
    trainer->setTiers(tiers);
}
