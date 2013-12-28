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

CONFIG(po_all): CONFIG += po_client po_clientplugins po_server po_serverplugins po_relaystation test

!CONFIG(po_server):!CONFIG(po_serverplugins):!CONFIG(po_registry):!CONFIG(po_relaystation):CONFIG += po_client
CONFIG(po_serverplugins):CONFIG += po_server
CONFIG(po_clientplugins):CONFIG += po_client
CONFIG(po_server):!CONFIG(po_norelaystation):CONFIG+= po_relaystation

CONFIG(po_client) | CONFIG(po_server) | CONFIG(po_registry) | CONFIG(po_relaystation) {
    SUBDIRS += src/Utilities
}

CONFIG(po_client) | CONFIG(po_server) | CONFIG(po_relaystation) {
    SUBDIRS += src/PokemonInfo
}

CONFIG(po_client) | CONFIG(po_serverplugins) | CONFIG(po_relaystation) {
    SUBDIRS += src/BattleManager
}

CONFIG(po_relaystation) | CONFIG(po_serverplugins) {
    SUBDIRS += src/QtWebsocket #git://gitorious.org/qtwebsocket/qtwebsocket.git
}

CONFIG(po_client) {
    SUBDIRS += src/Teambuilder
}

CONFIG(po_clientplugins) {
    SUBDIRS += src/ThemeManager \
               src/CSSChanger \
               src/ClientScripting \
               src/SettingsPlugin \
               src/SmogonPlugin \
               src/DesignerPlugin \
               src/DatabaseEditor
}

CONFIG(po_server) {
    SUBDIRS += src/Server \
        src/BattleServer
}

CONFIG(po_relaystation) {
    SUBDIRS += src/QJson \
               src/RelayStation
}

CONFIG(po_serverplugins) {
    SUBDIRS += src/UsageStatistics \
               src/StatsExtracter \
               src/BattleLogs \
               src/WebServerPlugin
}

CONFIG(po_registry) {
    CONFIG(webconf) {
        SUBDIRS += lib/pillow
    }
    SUBDIRS += src/Registry
}

TRANSLATIONS = src/trans/translation_de.ts \
    src/trans/translation_es.ts \
    src/trans/translation_fi.ts \
    src/trans/translation_fr.ts \
    src/trans/translation_he.ts \
    src/trans/translation_it.ts \
    src/trans/translation_jp.ts \
    src/trans/translation_ko.ts \
    src/trans/translation_nl.ts \
    src/trans/translation_pt-br.ts \
    src/trans/translation_zh-cn.ts

CONFIG(test) {
    SUBDIRS += \
        tests/utilities \
        tests/pokemoninfo \
        tests/battleserver \
        tests/server
}

contains(QT_VERSION, ^5\\.[1]\\..*):cache()

message(Following modules will be built: $$SUBDIRS)

