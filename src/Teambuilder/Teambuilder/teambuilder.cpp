#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/movesetchecker.h"
#include "teambuilder.h"
#include "poketablemodel.h"
#include "pokemovesmodel.h"
#include "box.h"
#include "mainwindow.h"
#include "pokedex.h"
#include "dockinterface.h"
#include "theme.h"
#include "modelenum.h"
#include "trainerbody.h"
#include "teambody.h"
#include "teamimporter.h"

/***********************************/
/**** TEAMBUILDER ******************/
/***********************************/


TeamBuilder::TeamBuilder(TrainerTeam *pub_team) : m_teamBody(NULL), m_boxes(NULL), m_pokedex(NULL), m_team(pub_team)
{
    for (int i = 0; i < NUMBER_GENS; i++)
        gens[i] = NULL;

    qRegisterMetaType<Pokemon::uniqueId>("Pokemon::uniqueId");

    setAttribute(Qt::WA_DeleteOnClose, true);

    setWindowTitle(tr("Teambuilder"));

    memset(modified,false,6);

    QVBoxLayout *vl =  new QVBoxLayout(this);
    vl->setSpacing(0);
    vl->setMargin(2);

    QHBoxLayout *upButtons = new QHBoxLayout();
    upButtons->setMargin(0);
    upButtons->setSpacing(0);
    vl->addLayout(upButtons, 54);

    /* Buttons of pokemons / trainers */
    QImageButton * m_trainer = Theme::Button("trainer");
    upButtons->addWidget(m_trainer,0,Qt::AlignTop);
    m_trainer->setAccessibleName(tr("Trainer"));

    QImageButton * m_team = Theme::Button("team");
    upButtons->addWidget(m_team,0,Qt::AlignTop);
    m_team->setAccessibleName(tr("Team"));

    QImageButton * m_box = Theme::Button("box");
    upButtons->addWidget(m_box,0,Qt::AlignTop);
    m_box->setAccessibleName(tr("Box"));

    QImageButton * m_pokedexb = Theme::Button("pokedex");
    upButtons->addWidget(m_pokedexb,0,Qt::AlignTop);
    m_pokedexb->setAccessibleName(tr("Pokedex"));

    currentZoneLabel = new QLabel();
    currentZoneLabel->setPixmap(Theme::Sprite("poketrainer"));
    upButtons->addWidget(currentZoneLabel,0, Qt::AlignTop);

    pokeModel = new PokeTableModel(gen(), this);

    /* Starting doing the "body" */
    m_body = new QStackedWidget(this);
    m_body->layout()->setMargin(0);

    /* Trainer body */
    m_trainerBody = new TB_TrainerBody(trainerTeam());
    m_body->addWidget(m_trainerBody);

    vl->addWidget(m_body, 585-2*54);

    QHBoxLayout *downButtons = new QHBoxLayout();

    QImageButton * m_new = Theme::Button("new");
    downButtons->addWidget(m_new, 0, Qt::AlignBottom);
    m_new->setAccessibleName(tr("New team"));

    QImageButton * m_load = Theme::Button("load");
    downButtons->addWidget(m_load, 0, Qt::AlignBottom);
    m_load->setAccessibleName(tr("Load team"));

    QImageButton * m_save = Theme::Button("save");
    downButtons->addWidget(m_save, 0, Qt::AlignBottom);
    m_save->setAccessibleName(tr("Save team"));

    QImageButton * m_close = Theme::Button("close");
    downButtons->addWidget(m_close, 0, Qt::AlignBottom);
    m_close->setAccessibleName(tr("Close teambuilder"));
    m_close->setAccessibleDescription(tr("Closes the teambuilder and applies the changes to the team"));

    downButtons->setMargin(0);
    downButtons->setSpacing(0);

    downButtons->addSpacing(currentZoneLabel->width());

    vl->addLayout(downButtons, 54);

    buttons[0] = m_trainer;
    buttons[1] = m_team;
    buttons[2] = m_box;
    buttons[3] = m_pokedexb;

    for (unsigned i = 0; i < sizeof(buttons)/sizeof(QImageButton*); i++) {
        buttons[i]->setCheckable(true);
    }

    m_trainer->setChecked(true);

    connect(m_trainer, SIGNAL(clicked()), SLOT(changeToTrainer()));
    connect(m_team, SIGNAL(clicked()), SLOT(changeToTeam()));
    connect(m_box, SIGNAL(clicked()), SLOT(changeToBoxes()));
    connect(m_pokedexb, SIGNAL(clicked()), SLOT(changeToPokedex()));
    connect(m_new, SIGNAL(clicked()), SLOT(newTeam()));
    connect(m_load, SIGNAL(clicked()), SLOT(loadTeam()));
    connect(m_save, SIGNAL(clicked()), SLOT(saveTeam()));
    connect(m_close, SIGNAL(clicked()), SIGNAL(done()));

    loadSettings(this, defaultSize());

    updateAll();
}

void TeamBuilder::initBox()
{
    m_boxes = new TB_PokemonBoxes(team());
    m_body->addWidget(m_boxes);

    connect(m_boxes, SIGNAL(pokeChanged(int)), SLOT(pokeChanged(int)));
}

void TeamBuilder::initPokedex()
{
    m_pokedex = new Pokedex(this, pokeModel);
    m_body->addWidget(m_pokedex);
}

void TeamBuilder::initTeam()
{
    m_teamBody = new TB_TeamBody(this, trainerTeam(), gen(), pokeModel);
    m_body->addWidget(m_teamBody);
}

int TeamBuilder::gen() const
{
    return team()->gen();
}

Team* TeamBuilder::team()
{
    return & m_team->team();
}

TrainerTeam * TeamBuilder::trainerTeam()
{
    return m_team;
}

Team* TeamBuilder::team() const
{
    return & m_team->team();
}

TrainerTeam * TeamBuilder::trainerTeam() const
{
    return m_team;
}

void TeamBuilder::pokeChanged(int poke)
{
    modified[poke] = true;
}

void TeamBuilder::changeZone()
{
    if (m_body->currentIndex() == TrainerW)
        changeToTeam();
    else
        changeToTrainer();
}

void TeamBuilder::genChanged() {
    int gen = sender()->property("gen").toInt();

    pokeModel->setGen(gen);

    if (m_teamBody) {
        m_teamBody->changeGeneration(gen);
    }
}

void TeamBuilder::changeToTeam()
{    if (m_teamBody == NULL) {
        initTeam();
    }
    buttons[m_body->currentIndex()]->setChecked(false);

    m_body->setCurrentWidget(m_teamBody);
    buttons[TeamW]->setChecked(true);

    currentZoneLabel->setPixmap(Theme::Sprite("poketeam"));

    for (int i = 0; i < 6; i++) {
        if (modified[i]) {
            m_teamBody->updatePoke(i);
            modified[i] = false;
        }
    }
}

void TeamBuilder::changeToBoxes()
{
    if (m_boxes == NULL) {
        initBox();
    }

    buttons[m_body->currentIndex()]->setChecked(false);

    m_body->setCurrentWidget(m_boxes);
    buttons[BoxesW]->setChecked(true);

    currentZoneLabel->setPixmap(Theme::Sprite("pokebox"));

    updateBox();
}

void TeamBuilder::changeToTrainer()
{
    if (m_body->currentWidget() != m_trainerBody) {
        buttons[m_body->currentIndex()]->setChecked(false);
        m_body->setCurrentWidget(m_trainerBody);

        buttons[TrainerW]->setChecked(true);
        currentZoneLabel->setPixmap(Theme::Sprite("poketrainer"));
    }
}

void TeamBuilder::changeToPokedex()
{
    if (m_pokedex == NULL) {
        initPokedex();
    }

    buttons[m_body->currentIndex()]->setChecked(false);

    m_body->setCurrentWidget(m_pokedex);

    currentZoneLabel->setPixmap(Theme::Sprite("pokedex"));
}

void TeamBuilder::saveTeam()
{
    saveTTeamDialog(*trainerTeam());
}

void TeamBuilder::loadTeam()
{
    loadTTeamDialog(*trainerTeam(), this, SLOT(updateAll()));
}

void TeamBuilder::newTeam()
{
    if (QMessageBox::question(this, tr("New Team"), tr("You sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        for (int i = 0; i < 6; i++) {
            team()->poke(i) = PokeTeam();
            team()->poke(i).setGen(trainerTeam()->team().gen());
        }
        updateTeam();
    }
}

void TeamBuilder::clickOnDone()
{
    emit done();
}

void TeamBuilder::updateAll()
{
    updateTrainer();
    updateTeam();
    updateBox();
}

void TeamBuilder::updateTeam()
{
    if (gens[team()->gen()-GEN_MIN]) {
        gens[team()->gen()-GEN_MIN]->setChecked(true);
    }

    if (m_teamBody) {
        m_teamBody->updateTeam();
    }
}

void TeamBuilder::updateTrainer()
{
    m_trainerBody->updateTrainer();
}

void TeamBuilder::updateBox()
{
    if (m_boxes) {
        m_boxes->updateBox();
    }
}

QMenuBar * TeamBuilder::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    menuBar->setObjectName("TeamBuilder");
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&New team"),this,SLOT(newTeam()),Qt::CTRL+Qt::Key_N);
    menuFichier->addAction(tr("&Save team"),this,SLOT(saveTeam()),Qt::CTRL+Qt::Key_S);
    menuFichier->addAction(tr("&Load team"),this,SLOT(loadTeam()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("&Import from text"),this,SLOT(importFromTxt()),Qt::CTRL+Qt::Key_I);
    menuFichier->addAction(tr("&Export to text"),this,SLOT(exportToTxt()),Qt::CTRL+Qt::Key_E);
    menuFichier->addAction(tr("&Quit"),qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);

    w->addStyleMenu(menuBar);
    w->addThemeMenu(menuBar);

    QMenu *gen = menuBar->addMenu(tr("&Gen."));
    QActionGroup *gens = new QActionGroup(gen);

    QString genStrings[] = {tr("Stadium (&1st gen)"), tr("GSC (&2nd gen)"), tr("Advance (&3rd gen)"),
                            tr("HGSS (&4th gen)"),tr("B/W (&5th gen)")};

    for (int i = 0; i < NUMBER_GENS; i++) {
        this->gens[i] = gen->addAction(genStrings[i], this, SLOT(genChanged()));
        this->gens[i]->setCheckable(true);
        this->gens[i]->setProperty("gen", i + GEN_MIN);
        gens->addAction(this->gens[i]);
    }

    this->gens[team()->gen()-GEN_MIN]->setChecked(true);

    QMenu *view = menuBar->addMenu(tr("&Options"));
    QAction *items = view->addAction(tr("&Show all items"));
    view->addAction(tr("&Full Screen (for netbook users ONLY)"), this, SLOT(showNoFrame()), Qt::Key_F11);
    QAction *forceMinLevels = view->addAction(tr("Enforce &minimum levels"));

    QSettings s;
    items->setCheckable(true);
    items->setChecked(s.value("show_all_items").toBool());
    forceMinLevels->setCheckable(true);
    forceMinLevels->setChecked(MoveSetChecker::enforceMinLevels);

    connect(items, SIGNAL(toggled(bool)), this, SLOT(changeItemDisplay(bool)));
    connect(forceMinLevels, SIGNAL(toggled(bool)), this, SLOT(enforceMinLevels(bool)));

    /* Loading mod menu */
    QSettings s_mod(PoModLocalPath + "mods.ini", QSettings::IniFormat);
    QStringList mods = s_mod.childGroups();
    modActionGroup = new QActionGroup(menuBar);
    if (mods.size() > 0) {
        int general_pos = mods.indexOf("General");
        if (general_pos != -1) {
            mods.removeAt(general_pos);
        }
        if (mods.size() > 0) {
            int mod_selected = s_mod.value("active", 0).toInt();
            bool is_mod_selected = mod_selected > 0;
            QMenu *menuMods = menuBar->addMenu(tr("&Mods"));

            // No mod option.
            QAction *action_no_mod = menuMods->addAction(tr("No mod"), this, SLOT(setNoMod()));
            action_no_mod->setCheckable(true);
            modActionGroup->addAction(action_no_mod);
            if (!is_mod_selected) action_no_mod->setChecked(true);
            menuMods->addSeparator();

            // Add mods to menu.
            QStringListIterator mods_it(mods);
            while (mods_it.hasNext()) {
                QString current = mods_it.next();
                QAction *ac = menuMods->addAction(current, this, SLOT(changeMod()));
                ac->setCheckable(true);
                if (is_mod_selected && (mod_selected == s_mod.value(current + "/id", 0).toInt())) {
                    ac->setChecked(true);
                }
                modActionGroup->addAction(ac);
            }
        }
    }

    return menuBar;
}

void TeamBuilder::changeMod()
{
    PokemonInfo::reloadMod(FillMode::Client, modActionGroup->checkedAction()->text());
    // TODO: MoveSetChecker::init("db/pokes/"); ?
}

void TeamBuilder::setNoMod()
{
    PokemonInfo::reloadMod(FillMode::Client);
}

void TeamBuilder::changeItemDisplay(bool b)
{
    QSettings s;
    s.setValue("show_all_items", b);

    if (m_teamBody) {
        for (int i = 0; i < 6; i++) {
            m_teamBody->reloadItems(b);
        }
    }
}

void TeamBuilder::enforceMinLevels(bool enforce)
{
    QSettings s;
    s.setValue("enforce_min_levels", enforce);
    MoveSetChecker::enforceMinLevels = enforce;
}

void TeamBuilder::showNoFrame()
{
    bool static k=false;//if it is full screen?
    if(k){
        topLevelWidget()->showNormal();
        k=false;
    }else{
        topLevelWidget()->showFullScreen();
        k=true;
    }
}

void TeamBuilder::importFromTxt()
{
    if (m_import) {
        m_import->raise();
        return;
    }
    m_import=new TeamImporter();
    m_import->show();
    connect(m_import, SIGNAL(done(QString)), SLOT(importDone(QString)));
}

void TeamBuilder::importDone(const QString &text)
{
    trainerTeam()->importFromTxt(text);
    updateTeam();
}

void TeamBuilder::exportToTxt()
{
    QTextEdit *exporting = new QTextEdit(this);
    exporting->setWindowFlags(Qt::Window);
    exporting->setAttribute(Qt::WA_DeleteOnClose, true);

    exporting->setText(trainerTeam()->exportToTxt());
    exporting->setReadOnly(true);

    exporting->show();
    exporting->setBackgroundRole(QPalette::Base);
    exporting->resize(500,700);
}

void TeamBuilder::setTierList(const QStringList &tiers)
{
    m_trainerBody->setTierList(tiers);
}

TeamBuilder::~TeamBuilder()
{
    writeSettings(this);
}
