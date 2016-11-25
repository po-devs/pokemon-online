# For building Pokemon Online
#
# Use
# qmake "CONFIG += po_client" to build client.
#       "CONFIG += po_server" to build server.
#       "CONFIG += po_all"    to build both client and server.
#
# By default only client will be build.
# It should be noted that PO still contains more modules.
#
# Shadow building can be enabled with "CONFIG += shadow"
#
# If you need to edit global qmake constants use
# src/Shared/Common.pri for that.

# Websocket library cloned from git://gitorious.org/qtwebsocket/qtwebsocket.git


TEMPLATE = subdirs
CONFIG += ordered

CONFIG(po_all): CONFIG += po_client po_clientplugins po_server po_serverplugins po_relaystation po_movemachine test

!CONFIG(po_server):!CONFIG(po_serverplugins):!CONFIG(po_registry):!CONFIG(po_relaystation):CONFIG += po_client
CONFIG(po_serverplugins):CONFIG += po_server
CONFIG(po_clientplugins):CONFIG += po_client
CONFIG(po_server):!CONFIG(po_norelaystation):CONFIG+= po_relaystation

SUBDIRS += src/libraries

CONFIG(po_client) {
    SUBDIRS += src/Teambuilder
}

CONFIG(po_movemachine) {
    SUBDIRS += src/MoveMachine
}

CONFIG(po_clientplugins) {
    SUBDIRS += src/client-plugins
}

CONFIG(po_server) {
    SUBDIRS += src/Server \
        src/BattleServer
}

CONFIG(po_relaystation) {
    SUBDIRS += src/RelayStation
}

CONFIG(po_serverplugins) {
    SUBDIRS += src/server-plugins
}

CONFIG(po_registry) {
    SUBDIRS += src/Registry
}

TRANSLATIONS = src/trans/translation_de.ts \
    src/trans/translation_es.ts \
    src/trans/translation_fr.ts \
    src/trans/translation_it.ts \
    src/trans/translation_zh-cn.ts
#The following languages were once supported but haven't been updated in awhile
#They are massively out of date and some didn't have anything translated for them!
    #src/trans/outdated/translation_fi.ts \
    #src/trans/outdated/translation_he.ts \
    #src/trans/outdated/translation_jp.ts \
    #src/trans/outdated/translation_ko.ts \
    #src/trans/outdated/translation_nl.ts \
    #src/trans/outdated/translation_pt-br.ts

CONFIG(test) {
    SUBDIRS += \
        tests
}

contains(QT_VERSION, ^5\\.[1]\\..*):cache()

message(Following modules will be built: $$SUBDIRS)


