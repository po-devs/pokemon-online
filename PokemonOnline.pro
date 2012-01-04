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

TEMPLATE = subdirs
CONFIG += ordered

CONFIG(po_all):CONFIG += po_client po_clientplugins po_server po_serverplugins

!CONFIG(po_server):!CONFIG(po_serverplugins):CONFIG += po_client
CONFIG(po_serverplugins):CONFIG += po_server

CONFIG(po_client) | CONFIG(po_server) {
    SUBDIRS = src/Utilities \
              src/PokemonInfo
}

CONFIG(po_client) {
    SUBDIRS += src/BattleManager \
               src/Teambuilder
}

CONFIG(po_clientplugins) {
    SUBDIRS += src/ThemeManager \
               src/CSSChanger \
               src/QRCodePlugin
}

CONFIG(po_server) {
    SUBDIRS += src/Server
}

CONFIG(po_serverplugins) {
    SUBDIRS += src/UsageStatistics \
               src/StatsExtracter \
               src/BattleManager \
               src/BattleLogs
}

macx:QMAKE_CC=echo

message(Following modules will be build: $$SUBDIRS)
