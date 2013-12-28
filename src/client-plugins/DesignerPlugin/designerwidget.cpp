#include "designerplugin.h"
#include "designerwidget.h"
#include "../Utilities/functions.h"

#include <QSettings>

DesignerWidget::DesignerWidget(DesignerPlugin *plugin) :
    QDialog(0),
    ui(new Ui::DesignerWidget),
    plugin(plugin)
{
    ui->setupUi(this);

    /* Loading state */
    QSettings s;

    liveReload = s.value("DesignerWidget/LiveReload", true).toBool();
    ui->liveReload->setChecked(liveReload);

    /* Updating UI with custom classes */
    loadSettings(this, QSize(655, 604));
    updateUi();

    /* Signals/slots */
    connect(ui->liveReload, SIGNAL(toggled(bool)), this, SLOT(liveReloadChanged(bool)));
    connect(ui->reload, SIGNAL(pressed()), this, SLOT(reloadPressed()));

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
}

DesignerWidget::~DesignerWidget()
{
    writeSettings(this);
    delete ui;
}

void DesignerWidget::updateUi()
{
    /* Update with a poketextedit */
    ui->infoOutput->deleteLater();
    ui->infoOutput = plugin->client->getPokeTextEdit(ui->infoTab);
    ui->infoOutput->setObjectName("infoOutput");
    ui->infoLayout->insertWidget(1, ui->infoOutput);

    ui->descOutput->deleteLater();
    ui->descOutput = plugin->client->getPokeTextEdit(ui->descTab);
    ui->descOutput->setObjectName("descOutput");
    ui->descLayout->insertWidget(1, ui->descOutput);

    ui->annOutput->deleteLater();
    ui->annOutput = plugin->client->getPokeTextEdit(ui->annTab);
    ui->annOutput->setObjectName("annOutput");
    ui->annLayout->insertWidget(1, ui->annOutput);

    QString profileInfo = plugin->client->trainerTeam()->profile().info().info;
    if (profileInfo.trimmed().length() > 0) {
        ui->infoInput->setPlainText(profileInfo);
    }

    ui->infoOutput->show();
    ui->descOutput->show();
    ui->annOutput->show();

    connect(ui->infoInput, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(ui->descInput, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(ui->annInput, SIGNAL(textChanged()), this, SLOT(textChanged()));

    /* Update everything */
    tabChanged(0);
}

/* Slots */
void DesignerWidget::liveReloadChanged(bool checked)
{
    QSettings s;
    s.setValue("DesignerWidget/LiveReload", checked);

    liveReload = checked;
}

void DesignerWidget::textChanged()
{
    QString plainText = tabInput()->toPlainText();
    int length = plainText.length();

    ui->charCount->setText(
                QString("<font color='%1'><b>%2</b></font>/%3 %4").arg(
                    (length > limitForTab ? "red" : "green"),
                    QString::number(length),
                    QString::number(limitForTab),
                    tr("characters")
                    )
                );

    if (liveReload && length <= limitForTab) {
        tabOutput()->setHtml(plainText);
    }
}

void DesignerWidget::tabChanged(int id)
{
    tab = id;
    limitForTab = limitPerTab[tab];

    textChanged();
}

void DesignerWidget::reloadPressed()
{
    QString plainText = tabInput()->toPlainText();
    if (plainText.length() <= limitForTab) {
        tabOutput()->setHtml(plainText);
    }
}
