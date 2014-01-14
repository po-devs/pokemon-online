TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += Utilities \
    PokemonInfo

CONFIG(po_all) {
    CONFIG += po_server po_registry po_client
}

!CONFIG(po_server):!CONFIG(po_registry):CONFIG+=po_client

CONFIG(po_client) {
    SUBDIRS += BattleManager \
        Teambuilder
}

CONFIG(po_server) {
    SUBDIRS += Server
}

CONFIG(po_registry) {
    SUBDIRS += Registry
}
