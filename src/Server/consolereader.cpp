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
            try {
                m_Server->pluginManager->addPlugin(line.section(" ", 1));
            } catch (std::runtime_error &ex) {
                m_Server->printLine(ex.what(), false, true);
            }
        } else if (line.indexOf("removep ") == 0) {
            if (!m_Server->pluginManager->freePlugin(line.section(" ", 1))) {
                m_Server->printLine("Error removing plugin '" + line.section(" ", 1) + "'", false, true);
            }
        } else if (line == "listp") {
            m_Server->printLine("Plugins: " + m_Server->pluginManager->getPlugins().join(", "), false, true);
        }
        else {
            m_Server->sendServerMessage(line);
        }
    }
}
