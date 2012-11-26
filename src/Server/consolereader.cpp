#include <iostream>

#include "server.h"
#include "pluginmanager.h"
#include "consolereader.h"

ConsoleReader::ConsoleReader(Server* server) : m_Server(server), m_TextStream(stdin)
{
}

void ConsoleReader::read(int)
{ 
    QString line = m_TextStream.readLine();
    if(line != "")
    {
        /* Add a plugin */
        if (line.indexOf("addp ") == 0) {
            m_Server->pluginManager->addPlugin(line.section(" ", 1));
        } else if (line.indexOf("removep ") == 0) {
            m_Server->pluginManager->freePlugin(line.section(" ", 1).toInt());
        } else if (line == "listp") {
            m_Server->printLine("Plugins: " + m_Server->pluginManager->getPlugins().join(", "), false, true);
        }
        else {
            m_Server->sendServerMessage(line);
        }
    }
}
