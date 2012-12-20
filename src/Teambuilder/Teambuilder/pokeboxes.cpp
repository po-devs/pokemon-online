#include "pokeboxes.h"
#include "pokebox.h"
#include "ui_pokeboxes.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "Teambuilder/teamholder.h"
#include "theme.h"

PokeBoxes::PokeBoxes(QWidget *parent, TeamHolder *nteam) :
    TeamBuilderWidget(parent), m_team(nteam),
    ui(new Ui::PokeBoxes)
{
    ui->setupUi(this);

    //ui->trainerHome->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowBack));
    ui->pokemonButtons->setTeam(team().team());
    changePoke(&team().team().poke(0), 0);
    updatePoke();

    loadBoxes();

    //connect(ui->pokemonButtons, SIGNAL(doubleClicked(int)), SLOT(changeTeamPoke(int)));
    connect(ui->pokemonButtons, SIGNAL(teamChanged()), SIGNAL(teamChanged()));
    connect(ui->pokemonButtons, SIGNAL(dropEvent(int,QDropEvent*)), SLOT(dealWithButtonDrop(int, QDropEvent*)));
    connect(ui->pokemonButtons, SIGNAL(clicked(int)), SLOT(changeTeamPoke(int)));
    connect(ui->storeButton, SIGNAL(clicked()), SLOT(storePokemon()));
    connect(ui->deleteButton, SIGNAL(clicked()), SLOT(deletePokemon()));
    connect(ui->withdrawButton, SIGNAL(clicked()), SLOT(withdrawPokemon()));
    connect(ui->switchButton, SIGNAL(clicked()), SLOT(switchPokemon()));
    connect(ui->boxes, SIGNAL(currentChanged(int)), SLOT(currentBoxChanged(int)));
    connect(ui->addBox, SIGNAL(clicked()), SLOT(newBox()));
    connect(ui->editBoxName, SIGNAL(clicked()), SLOT(editBoxName()));
    connect(ui->deleteBox, SIGNAL(clicked()), SLOT(deleteBox()));
    connect(ui->boxes, SIGNAL(tabCloseRequested(int)), SLOT(deleteBox(int)));
    //connect(ui->trainerHome, SIGNAL(clicked()), SIGNAL(done()));
}

PokeBoxes::~PokeBoxes()
{
    delete ui;
}

void PokeBoxes::currentBoxChanged(int b)
{
    boxes[b]->loadIfNeeded();
}

void PokeBoxes::showPoke(PokeTeam *poke)
{
    PokeBox *box = qobject_cast<PokeBox*>(sender());

    if (box) {
        changePoke(poke, box->currentSlot(), box->getNum());
    } else {
        changePoke(poke);
    }
    updatePoke();
}

void PokeBoxes::updateTeam()
{
    ui->pokemonButtons->setTeam(team().team());

    if (displayedBox == -1) {
        changePoke(&team().team().poke(displayedSlot));
    }
    updatePoke();
}

void PokeBoxes::changePoke(PokeTeam *poke, int slot, int box)
{
    this->displayedBox = box;
    if (slot != -1) {
        this->displayedSlot = slot;
    }
    this->m_poke = poke;
}

void PokeBoxes::updatePoke()
{
    if (displayedBox != -1) {
        ui->boxInfoLabel->setText(tr("Box %1 slot %2").arg(displayedBox+1).arg(displayedSlot+1));
    } else {
        ui->boxInfoLabel->setText(tr("Team slot %1").arg(displayedSlot+1));
    }
    ui->nickNameLabel->setText(poke().nickname());
    ui->speciesLabel->setText(PokemonInfo::Name(poke().num()));
    ui->pokemonSprite->setPixmap(poke().picture());
    ui->pokemonSprite->setFixedSize(poke().picture().size());
    ui->type1Label->setPixmap(Theme::TypePicture(PokemonInfo::Type1(poke().num(), poke().gen())));
    if(PokemonInfo::Type2(poke().num(), poke().gen()) != Type::Curse) {
        ui->type2Label->setVisible(true);
        ui->type2Label->setPixmap(Theme::TypePicture(PokemonInfo::Type2(poke().num(), poke().gen())));
    } else {
        ui->type2Label->setVisible(false);
    }
    ui->nature->setText(NatureInfo::Name(poke().nature()));
    ui->itemSprite->setPixmap(ItemInfo::Icon(poke().item()));
    ui->genderLabel->setPixmap(Theme::GenderPicture(poke().gender()));
    ui->levelLabel->setText(tr("Lv. %1").arg(poke().level()));
    QString movesInfo;
    for(int movesCount = 0; movesCount < 4; movesCount++) {
        if(movesCount == 4) {
            movesInfo.append(QString("%1").arg(MoveInfo::Name(poke().move(movesCount))));
        } else {
            movesInfo.append(QString("%1\n").arg(MoveInfo::Name(poke().move(movesCount))));
        }
    }
    ui->moves->setText(movesInfo);
}

void PokeBoxes::changeTeamPoke(int index)
{
    changePoke(&team().team().poke(index), index);
    updatePoke();
}

void PokeBoxes::loadBoxes()
{
    QDir directory = QDir(PokeBox::getBoxPath());

    QStringList files = directory.entryList(QStringList() << "*.box", QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);

    if (files.size() == 0) {
        /* Tries to get old boxes */
        QApplication::setApplicationName("Pokeymon-Online");
        QDir dir = QDir(PokeBox::getBoxPath());
        QApplication::setApplicationName("Pokemon-Online");

        QStringList f = dir.entryList(QStringList() << "*.box", QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);

        if (f.length() != 0) {
            foreach(QString fs, f) {
                QFile x(fs);
                x.copy(directory.absoluteFilePath(fs));
            }
            files = f;
        } else {
            files << tr("Box%201.box") << tr("Box%202.box");
        }
    }

    foreach (QString file, files) {
        addBox(file);
    }
}

void PokeBoxes::switchBoxTeam(int box, int boxslot, int teamslot)
{
    ui->boxes->setCurrentIndex(box);
    currentBox()->changeCurrentSpot(boxslot);

    ui->pokemonButtons->setCurrentSlot(teamslot);

    try {
        currentBox()->getCurrent();
        if (team().team().poke(teamslot).num() == 0)
            withdrawPokemon();
        else
            switchPokemon();
    } catch (const QString&) {
        try {
            storePokemon();
        } catch (const QString&) {

        }
    }
}

void PokeBoxes::addBox(const QString &name)
{
    PokeBox *box = new PokeBox(boxes.size(), name);
    boxes.push_back(box);
    box->setParent(this);

    /* Always load the first box */
    if (boxes.size() == 1) {
        box->loadBox();
    }

    ui->boxes->addTab(box, box->getBoxName());

    connect(box, SIGNAL(switchWithTeam(int,int,int)), SLOT(switchBoxTeam(int,int,int)));
    connect(box, SIGNAL(show(PokeTeam*)), SLOT(showPoke(PokeTeam*)));
}

void PokeBoxes::newBox()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New Box"),
                                         tr("Enter the new name for the new box:"), QLineEdit::Normal,
                                          tr("New Box"), &ok);
    if (ok && !text.isEmpty() && !existBox(text)) {
        addBox(QString::fromUtf8(QUrl::toPercentEncoding(text))+".box");
    }
}

void PokeBoxes::editBoxName()
{
    bool ok;
    PokeBox *box = currentBox();
    QString text = QInputDialog::getText(this, tr("Edit Box Name"),
                                         tr("Enter the new name for the box %1:").arg(box->getBoxName()), QLineEdit::Normal,
                                          box->getBoxName(), &ok);
    if (ok && !text.isEmpty() && !existBox(text)) {
         box->reName(text);
         ui->boxes->setTabText(box->getNum(), text);
     }
}

void PokeBoxes::deleteBox(int num)
{
    if (boxes.size() <= 1)
        return;

    if (num == -1) {
        num = currentBox()->getNum();
    }

    int res = QMessageBox::question(this, tr("Destroying a box"), tr("Do you want to delete box %1 permanently?").arg(boxes[num]->getBoxName()), QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::Yes) {
        doDeleteBox(num);
    }
}

void PokeBoxes::dealWithButtonDrop(int index, QDropEvent *event)
{
    const QMimeData *data = event->mimeData();
    bool b1 = data->data("Box").isNull();
    bool b2 = data->data("Item").isNull();

    if(!b1 && !b2) {
        event->accept();

        PokeBox *box = (PokeBox*)(intptr_t)data->data("Box").toLongLong();
        int item = data->data("Item").toInt();

        switchBoxTeam(box->getNum(), item, index);
    }
}

void PokeBoxes::doDeleteBox(int num)
{
    boxes[num]->deleteBox();

    PokeBox *b = boxes[num];
    boxes.removeAt(num);

    for (int i = num; i < boxes.size(); i++) {
        boxes[i]->setNum(i);
    }

    delete b;
}

bool PokeBoxes::existBox(const QString &name) const
{
    foreach(PokeBox *box, boxes) {
        if (box->getBoxName() == name)
            return true;
    }

    return false;
}

void PokeBoxes::storePokemon()
{
    try {
        currentBox()->addPokemonToBox(team().team().poke(currentPoke()));
        currentBox()->saveBox();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box %1 - %2").arg(currentBox()->getNum()).arg(currentBox()->getBoxName()), ex);
    }
}

void PokeBoxes::withdrawPokemon()
{
    try {
        setCurrentTeamPoke(currentBox()->getCurrent());
        updateSpot(currentPoke());
        emit teamChanged();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box %1 - %2").arg(currentBox()->getNum()).arg(currentBox()->getBoxName()), ex);
    }
}

void PokeBoxes::switchPokemon()
{
    try {
        PokeTeam *p = new PokeTeam(*currentBox()->getCurrent());
        currentBox()->changeCurrent(*currentPokeTeam());
        currentBox()->saveBox();
        setCurrentTeamPoke(p);

        /* Don't worry, if getCurrent doesn't throw exceptions then changeCurrent doesn't.
           Hence no memory leaks */
        delete p;

        updateSpot(currentPoke());
        teamChanged();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box %1 - %2").arg(currentBox()->getNum()).arg(currentBox()->getBoxName()), ex);
    }
}

void PokeBoxes::deletePokemon()
{
    try {
        currentBox()->deleteCurrent();
        currentBox()->saveBox();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box %1 - %2").arg(currentBox()->getNum()).arg(currentBox()->getBoxName()), ex);
    }
}

int PokeBoxes::currentPoke() const
{
    return std::max(ui->pokemonButtons->currentSlot(), 0);
}

PokeBox *PokeBoxes::currentBox()
{
    return boxes[ui->boxes->currentIndex()];
}

void PokeBoxes::updateSpot(int i)
{
    ui->pokemonButtons->updatePoke(i);
}

void PokeBoxes::setCurrentTeamPoke(PokeTeam *p)
{
    *currentPokeTeam() = *p;

    if (p->gen() != team().team().gen()) {
        p->setGen(team().team().gen());
        p->runCheck();
    }
}

PokeTeam *PokeBoxes::currentPokeTeam()
{
    return &team().team().poke(currentPoke());
}

