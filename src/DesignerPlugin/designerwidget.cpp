#include "designerplugin.h"
#include "designerwidget.h"
#include "ui_designerwidget.h"
#include "../Utilities/functions.h"

#include <QSettings>

DesignerWidget::DesignerWidget(DesignerPlugin *plugin) :
    QDialog(0),
    ui(new Ui::DesignerWidget),
    plugin(plugin)
{
    ui->setupUi(this);
    updateUi();

    loadSettings(this, QSize(655, 604));

    /* Loading state */
    QSettings s;

    liveReload = s.value("DesignerWidget/LiveReload", true).toBool();
    ui->liveReload->setChecked(liveReload);

    /* Signals/slots */
    connect(ui->liveReload, SIGNAL(toggled(bool)), this, SLOT(liveReloadChanged(bool)));
    connect(ui->reload, SIGNAL(pressed()), this, SLOT(reloadPressed()));

    connect(ui->infoInput, SIGNAL(textChanged()), this, SLOT(infoTextChanged()));
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
    ui->infoOutput = plugin->client->getPokeTextEdit();
    ui->infoOutput->setParent(ui->infoTab);
    ui->infoOutput->setObjectName("infoOutput");
    ui->infoOutput->setGeometry(QRect(10, 280, 611, 241));
    ui->infoOutput->setHtml(tr("<i>HTML output here</i>."));
    ui->infoOutput->show();

    infoTextChanged();
}

void DesignerWidget::accept()
{
    // TODO: Save old info
    QDialog::accept();
}

/* Slots */
void DesignerWidget::liveReloadChanged(bool checked)
{
    QSettings s;
    s.setValue("DesignerWidget/LiveReload", checked);

    liveReload = checked;
}

void DesignerWidget::infoTextChanged()
{
    QString plainText = ui->infoInput->toPlainText();
    int limit = limitForTab();
    int length = plainText.length();

    ui->charCount->setText(
                QString("%1/%2 %3").arg(
                    QString("<font color='%1'><b>%2</b></font>").arg((length > limit ? "red" : "green"), QString::number(length)),
                    QString::number(limit),
                    tr("characters")
                    )
                );

    if (liveReload && length <= limit) {
        ui->infoOutput->setHtml(plainText);
    }
}

void DesignerWidget::reloadPressed()
{
    QString plainText = ui->infoInput->toPlainText();
    if (plainText.length() <= limitForTab()) {
        ui->infoOutput->setHtml(plainText);
    }
}


int DesignerWidget::limitForTab()
{
    int tab = ui->tabWidget->currentIndex();
    switch (tab) {
    case 0: /* trainer info */
        return 300;
    case 1: /* server description */
        return 500;
    default: /* 2 - server announcement */
        /* Could be of any length, but a fairly good guideline */
        return 80000;
    }
}
