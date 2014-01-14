#include "tierwindow.h"
#include "tiermachine.h"
#include "tier.h"
#include <Utilities/confighelper.h>

TierWindow::TierWindow(QWidget *parent) : QWidget(parent), helper(NULL)
{
    setAttribute(Qt::WA_DeleteOnClose,true);

    QGridLayout *layout = new QGridLayout(this);

    dataTree = TierMachine::obj()->getDataTree();

    m_tree = new QTreeWidget();
    layout->addWidget(m_tree, 0, 0, 2, 1);
    m_tree->header()->hide();
    m_tree->setFixedWidth(200);

    configWidget = new QWidget();
    layout->addWidget(configWidget, 0, 1, 1, 2);
    QVBoxLayout *v = new QVBoxLayout(configWidget);
    internalWidget = new QWidget();
    v->addWidget(internalWidget);

    QPushButton *add, *finish;
    add = new QPushButton("Add New...");

    finish = new QPushButton("Finish and Apply");

    layout->addWidget(add,1,1);
    layout->addWidget(finish,1,2);
    layout->setColumnStretch(1, 50);
    layout->setColumnStretch(2, 50);

    updateTree();

    connect(m_tree, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(editingRequested(QTreeWidgetItem*)));
    connect(add, SIGNAL(clicked()), SLOT(addNew()));
    connect(finish, SIGNAL(clicked()), SLOT(done()));
    connect(finish, SIGNAL(clicked()), SLOT(close()));
}

TierWindow::~TierWindow() {
    clearCurrentEdit();
    delete dataTree;
}

void TierWindow::addNew()
{
    TierWNameW *w = new TierWNameW();

    connect(w, SIGNAL(addNewCategory(QString)), SLOT(addNewCategory(QString)));
    connect(w, SIGNAL(addNewTier(QString)), SLOT(addNewTier(QString)));
}

void TierWindow::editingRequested(QTreeWidgetItem *item)
{
    clearCurrentEdit();

    TierNode *t = dataTree->getNode(item->text(0));
    if (!t)
        return;

    if ( t->isCategory() ) {
        openCategoryEdit((TierCategory*)t);
    } else {
        openTierEdit((Tier*)t);
    }
}

void TierWindow::clearCurrentEdit()
{
    delete helper, helper = NULL;
    currentEdit.clear();

    if (internalWidget) {
        configWidget->layout()->removeWidget(internalWidget);
        delete internalWidget, internalWidget = NULL;
    }
}

void TierWindow::done()
{
    TierMachine::obj()->fromString(dataTree->toXml());
    TierMachine::obj()->save();
}

void TierWindow::openCategoryEdit(TierCategory *c)
{
    helper = new ConfigForm("Delete Category", "Apply", this);
    currentEdit = c->name();
    currentType = CategoryT;
    currentTierCat = c;

    helper->addConfigHelper(new ConfigLine("Name", currentEdit));

    {
        QStringList parents;
        parents << "";
        QList<TierCategory *> cats = dataTree->gatherCategories();
        foreach(TierCategory *c, cats) {
            parents << c->name();
        }
        parent = dataTree->getParentCategory(c)->name();
        helper->addConfigHelper(new ConfigCombo<QString>("Parent Category", parent, parents, parents));
    }

    helper->addConfigHelper(new ConfigSpin("Category display order", c->displayOrder, -100, 100));

    internalWidget = helper->generateConfigWidget();
    updateInternalWidget();

    connect(helper, SIGNAL(button1()), SLOT(deleteCurrent()));
    connect(helper, SIGNAL(button2()), SLOT(updateCategory()));
}

void TierWindow::openTierEdit(Tier *t)
{
    helper = new ConfigForm("Delete Tier", "Apply", this);
    currentEdit = t->name();
    currentType = TierT;
    currentTier = t;

    helper->addConfigHelper(new ConfigLine("Name", currentEdit));

    {
        QStringList parents;
        parents << "";
        QList<TierCategory *> cats = dataTree->gatherCategories();
        foreach(TierCategory *c, cats) {
            parents << c->name();
        }
        parent = dataTree->getParentCategory(t)->name();
        helper->addConfigHelper(new ConfigCombo<QString>("Parent Category", parent, parents, parents));
    }
    {
        QStringList parents;
        parents << "";
        QList<Tier *> tiers = dataTree->gatherTiers();
        foreach(Tier *t, tiers) {
            parents << t->name();
        }
        helper->addConfigHelper(new ConfigCombo<QString>("Parent Tier", t->banParentS, parents, parents));
    }

    QStringList genS = QStringList() << "Any";
    QList<Pokemon::gen> gens;
    gens.push_back(0);
    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++) {
        for (int j = 0; j < GenInfo::NumberOfSubgens(i); j++) {
            gens << Pokemon::gen(i, j);
            genS << QString("%1 (%2)").arg(GenInfo::Gen(gens.back().num), GenInfo::Version(gens.back()));
        }
    }

    helper->addConfigHelper(new ConfigCombo<Pokemon::gen>("Generation", t->m_gen, genS,  gens ));
    helper->addConfigHelper(new ConfigCheck("Ban pokemon/moves/items (uncheck to restrict the choice to them instead)", t->banPokes));
    helper->addConfigHelper(new ConfigSpin("Max number of pokemon", t->numberOfPokemons, 1, 6));
    helper->addConfigHelper(new ConfigText("Pokemon", pokemons));
    helper->addConfigHelper(new ConfigLine("Moves", moves));
    helper->addConfigHelper(new ConfigLine("Items", items));
    helper->addConfigHelper(new ConfigSpin("Max number of restricted pokemon", t->maxRestrictedPokes, 0, 6));
    helper->addConfigHelper(new ConfigLine("Restricted Pokemon", restrPokemons));
    helper->addConfigHelper(new ConfigSpin("Pokemon's max level", t->maxLevel, 1, 100));

    pokemons = t->getBannedPokes();
    moves = t->getBannedMoves();
    items = t->getBannedItems();
    restrPokemons = t->getRestrictedPokes();

    helper->addConfigHelper(new ConfigCombo<int>("Battle Mode in Find Battle", t->mode, QStringList() << "Singles" << "Doubles" << "Triples"
                                                 << "Rotation Battles",
                                                 QList<int>() << ChallengeInfo::Singles << ChallengeInfo::Doubles << ChallengeInfo::Triples
                                                 << ChallengeInfo::Rotation));

    int clauses = t->clauses;
    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        this->clauses[i] = (clauses >> i) % 2;

        helper->addConfigHelper(new ConfigCheck(ChallengeInfo::clause(i), this->clauses[i]));
    }

    helper->addConfigHelper(new ConfigSpin("Tier display order", t->displayOrder, -100, 100));

    internalWidget = helper->generateConfigWidget();
    internalWidget->layout()->setMargin(0);
    internalWidget->layout()->setSpacing(0);
    updateInternalWidget();

    connect(helper, SIGNAL(button1()), SLOT(deleteCurrent()));
    connect(helper, SIGNAL(button2()), SLOT(updateTier()));
}

void TierWindow::deleteCurrent()
{
    QMessageBox::StandardButton answer = QMessageBox::question(this, "Deletion of " + currentEdit,
        QString("Are you sure you want to delete the tier/category %1? If you do so, the data will be definitely lost, and if it's a category all subtiers/categories will be lost too.")
        .arg(currentEdit), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (answer == QMessageBox::Yes) {
        TierNode *n;
        if (currentType == TierT)
            n = currentTier;
        else
            n = currentTierCat;
        TierCategory *parent = dataTree->getParentCategory(n);
        parent->kill(n);
        clearCurrentEdit();
        updateTree();
    }
}

void TierWindow::addNewCategory(const QString &name)
{
    TierNode *t = dataTree->getNode(name);

    if (t) {
        QMessageBox::information(this, "Name Taken", "The name is already taken, the category won't be created");
        return;
    }

    TierCategory *c = new TierCategory();
    c->changeName(name);

    dataTree->root.appendChild(c);

    updateTree();
}

void TierWindow::addNewTier(const QString &name)
{
    TierNode *n = dataTree->getNode(name);

    if (n) {
        QMessageBox::information(this, "Name Taken", "The name is already taken, the tier won't be created");
        return;
    }

    Tier *t = new Tier();
    t->changeName(name);

    dataTree->root.appendChild(t);

    updateTree();
}

void TierWindow::updateTier()
{
    helper->applyVals();

    if (currentEdit != currentTier->name()) {
        /* Prevent to have duplicate names */
        if (dataTree->getNode(currentEdit)) {
            currentEdit = currentTier->name();
            QMessageBox::information(this, "Name Taken", "The name is already taken, so the old one will stay");
        } else {
            currentTier->changeName(currentEdit);
        }
    }

    currentTier->importBannedItems(items);
    currentTier->importBannedPokes(pokemons);
    currentTier->importBannedMoves(moves);
    currentTier->importRestrictedPokes(restrPokemons);

    int clRes = 0;
    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        if (clauses[i])
            clRes |= 1 << i;
    }
    currentTier->clauses = clRes;

    TierCategory *c = dataTree->getParentCategory(currentTier);
    if (c->name() != parent) {
        c->removeChild(currentTier);
        /* If the xml file is malformed, and has a tier and a category with the same name, it can crash here */
        TierCategory *c = (TierCategory*)dataTree->getNode(parent);
        c->appendChild(currentTier);
    }

    updateTree();
}

void TierWindow::updateCategory()
{
    helper->applyVals();

    if (currentEdit != currentTierCat->name()) {
        /* Prevent to have duplicate names */
        if (dataTree->getNode(currentEdit)) {
            currentEdit = currentTierCat->name();
            QMessageBox::information(this, "Name Taken", "The name is already taken, so the old one will stay");
        } else {
            currentTierCat->changeName(currentEdit);
        }
    }

    TierCategory *c = dataTree->getParentCategory(currentTierCat);
    if (c->name() != parent && currentTierCat->name() != parent) {
        c->removeChild(currentTierCat);
        /* If the xml file is malformed, and has a tier and a category with the same name, it can crash here */
        TierCategory *c = (TierCategory*)dataTree->getNode(parent);
        c->appendChild(currentTierCat);
    }

    updateTree();
}

void TierWindow::updateTree()
{
    dataTree->reorder();
    dataTree->buildTreeGui(m_tree);
    m_tree->expandAll();
}

void TierWindow::updateInternalWidget()
{
    if (internalWidget) {
        configWidget->layout()->addWidget(internalWidget);
    }
}

TierWNameW::TierWNameW()
{
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *v = new QVBoxLayout(this);
    v->addWidget(line = new QLineEdit());
    v->addWidget(bTier = new QRadioButton("Tier"));
    v->addWidget(bCat = new QRadioButton("Category"));
    QPushButton *ok = new QPushButton("Ok");
    v->addWidget(ok);

    connect(ok, SIGNAL(clicked()), SLOT(addClicked()));
    connect(ok, SIGNAL(clicked()), SLOT(close()));

    show();
}

void TierWNameW::addClicked()
{
    if (!bTier->isChecked() && !bCat->isChecked())
        return;
    QString text = line->text();

    if (text.length() == 0) {
        return;
    }

    if (bTier->isChecked()) {
        emit addNewTier(text);
    } else {
        emit addNewCategory(text);
    }
}
