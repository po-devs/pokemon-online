#ifndef SERVERPLUGININTERFACE_H
#define SERVERPLUGININTERFACE_H

#include "../Utilities/plugininterface.h"

/* An interface Server Plugins must follow.
   Remember that plugins are still very experimental and that this file is going to be subject to
   a lot of changes... */

class ServerInterface;

/* See ../Utilities/plugininterface.h for the class def */
class ServerPlugin : public Plugin
{
};

/* Each plugin will have to have a function like that named
   createPluginClass, that creates a ServerEngine (or a derived
    class) through new and returns it. */
typedef ServerPlugin *(*ServerPluginInstanceFunction) (ServerInterface *);

/* Will be used like that:

class MyPlugin : public ServerPlugin
{
...
}

extern "C" {
ServerPlugin *createServerPlugin(ServerInterface*);
};

....

ServerPlugin *createServerPluginClass(ServerInterface *server) {
    return new MyPlugin(server);
}

*/

#endif // SERVERPLUGININTERFACE_H
