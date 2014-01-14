TEMPLATE = subdirs

CONFIG += ordered

CONFIG(po_all): CONFIG += po_client po_clientplugins po_server po_serverplugins po_relaystation test

!CONFIG(po_server):!CONFIG(po_serverplugins):!CONFIG(po_registry):!CONFIG(po_relaystation):CONFIG += po_client
CONFIG(po_serverplugins):CONFIG += po_server
CONFIG(po_clientplugins):CONFIG += po_client
CONFIG(po_server):!CONFIG(po_norelaystation):CONFIG+= po_relaystation

CONFIG(po_client) | CONFIG(po_server) | CONFIG(po_registry) | CONFIG(po_relaystation) {
    SUBDIRS += Utilities
}

CONFIG(po_client) | CONFIG(po_server) | CONFIG(po_relaystation) {
    SUBDIRS += PokemonInfo
}

CONFIG(po_client) | CONFIG(po_serverplugins) | CONFIG(po_relaystation) {
    SUBDIRS += BattleManager
}

CONFIG(po_relaystation) | CONFIG(po_serverplugins) {
    SUBDIRS += QtWebsocket #git://gitorious.org/qtwebsocket/qtwebsocket.git
}

CONFIG(po_client) {
    SUBDIRS += TeambuilderLibrary
}

CONFIG(po_relaystation) {
    SUBDIRS += QJson
}

CONFIG(po_registry) {
    CONFIG(webconf) {
        SUBDIRS += ../../lib/pillow
    }
}
