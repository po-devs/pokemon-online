#include "tierwindow.h"
#include "tiermachine.h"
#include "confighelper.h"
#include "tier.h"

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
    add = new QPushButton("Add New");
    QMenu *m = new QMenu(add);
    m->addAction("Tier");
    m->addAction("Category");
    add->setMenu(m);

    finish = new QPushButton("Finish and Apply");

    layout->addWidget(add,1,1);
    layout->addWidget(finish,1,2);
    layout->setColumnStretch(1, 50);
    layout->setColumnStretch(2, 50);

    updateTree();

    connect(m_tree, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(editingRequested(QTreeWidgetItem*)));
    connect(finish, SIGNAL(clicked()), SLOT(done()));
    connect(finish, SIGNAL(clicked()), SLOT(close()));
}

TierWindow::~TierWindow() {
    clearCurrentEdit();
    delete dataTree;
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

    emit tiersChanged();
}

void TierWindow::openCategoryEdit(TierCategory *)
{

}

void TierWindow::openTierEdit(Tier *t)
{
    helper = new ConfigForm("Delete Tier", "Apply");
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

    helper->addConfigHelper(new ConfigCombo<int>("Generation", t->gen, QStringList() << "Any" << "3rd Gen" << "4th Gen",
                            QList<int> () << 0 << 3 << 4));
    helper->addConfigHelper(new ConfigCheck("Ban pokemons/moves/items (uncheck to restrict the choice to them instead)", t->banPokes));
    helper->addConfigHelper(new ConfigSpin("Max number of pokemons", t->numberOfPokemons, 1, 6));
    helper->addConfigHelper(new ConfigText("Pokemons", pokemons));
    helper->addConfigHelper(new ConfigLine("Moves", moves));
    helper->addConfigHelper(new ConfigLine("Items", items));
    helper->addConfigHelper(new ConfigSpin("Max number of restricted pokemons", t->maxRestrictedPokes, 0, 6));
    helper->addConfigHelper(new ConfigLine("Restricted Pokemons", restrPokemons));
    helper->addConfigHelper(new ConfigSpin("Max level of pokemons", t->maxLevel, 1, 100));
    helper->addConfigHelper(new ConfigSpin("Tier display order", t->displayOrder, -100, 100));

    pokemons = t->getBannedPokes();
    moves = t->getBannedMoves();
    items = t->getBannedItems();
    restrPokemons = t->getRestrictedPokes();

    helper->addConfigHelper(new ConfigCombo<int>("Doubles in Find Battle", t->doubles, QStringList() << "Force" << "Allow" << "Forbid",
                                                 QList<int>() << 1 << 0 << (-1) ));

    internalWidget = helper->generateConfigWidget();
    internalWidget->layout()->setMargin(0);
    internalWidget->layout()->setSpacing(0);
    updateInternalWidget();

    connect(helper, SIGNAL(button2()), SLOT(updateTier()));
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

    TierCategory *c = dataTree->getParentCategory(currentTier);
    if (c->name() != parent) {
        c->removeChild(currentTier);
        /* If the xml file is malformed, and has a tier and a category with the same name, it can crash here */
        TierCategory *c = (TierCategory*)dataTree->getNode(parent);
        c->appendChild(currentTier);
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
