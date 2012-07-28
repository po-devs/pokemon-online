TEMPLATE = subdirs

CONFIG += ordered po_server

include("core.pro")

SUBDIRS +=  BattleManager\
               UsageStatistics \
               StatsExtracter \
               BattleLogs
