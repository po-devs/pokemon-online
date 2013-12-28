#include "pokemoneditordialog.h"
#include "databaseeditor.h"

ClientPlugin *createClientPlugin(MainEngineInterface *client)
{
    return new DatabaseEditor(client);
}


DatabaseEditor::DatabaseEditor(MainEngineInterface *client)
{
    (void) client;
}

QString DatabaseEditor::pluginName() const
{
    return "Database Editor";
}

bool DatabaseEditor::hasConfigurationWidget() const
{
    return true;
}

QWidget * DatabaseEditor::getConfigurationWidget()
{
    return new PokemonEditorDialog();
}
