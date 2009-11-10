#include "serverchoice.h"
#include "../Utilities/otherwidgets.h"

ServerChoice::ServerChoice()
{
    setInputMode(QInputDialog::TextInput);

    QSettings settings;
    setTextValue(settings.value("default_server").toString());

    setWindowTitle(tr("Server Choice"));

    setLabelText(tr("Enter the host name or IP of the server you want to go"));

    connect(this, SIGNAL(textValueSelected(QString)), SLOT(textSelected(QString)));
}

void ServerChoice::textSelected(const QString &text)
{
    /* Just storing the new default server */
    QSettings settings;
    settings.setValue("default_server", text);
}
