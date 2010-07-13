#include <iostream>

#include "server.h"
#include "consolereader.h"

ConsoleReader::ConsoleReader(Server* server) : m_Server(server), m_TextStream(stdin)
{
}

void ConsoleReader::read(int)
{ 
    QString line = m_TextStream.readLine();
    if(line != "")
    {
        m_Server->sendServerMessage(line);
    }
}
