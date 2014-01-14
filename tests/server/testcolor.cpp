#include <QColor>

#include <PokemonInfo/teamholder.h>
#include <Teambuilder/analyze.h>

#include "testcolor.h"

void TestColor::onPlayerConnected()
{
    sender()->login(TeamHolder("Rainbow"), false, false, QColor("#e37415"));
    sender()->sendChanMessage(0, "eval: '<color>: ' + sys.getColor(sys.id('Rainbow'))");
}

void TestColor::onChannelMessage(const QString &message, int, bool)
{
    if (message.startsWith("<color>: ")) {
        if (message == "<color>: #e37415") {
            sender()->sendChanMessage(0, "eval: sys.changeColor(sys.id('Rainbow'), '#e97e24')");
            sender()->sendChanMessage(0, "eval: '<color>: ' + sys.getColor(sys.id('Rainbow'))");
        } else if (message == "<color>: #e97e24") {
            accept();
        }
    }
}
