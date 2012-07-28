TEMPLATE = subdirs

CONFIG += ordered po_client

include("core.pro")

SUBDIRS += ThemeManager \
               CSSChanger \
               QRCodePlugin \
               ClientScripting \
               SettingsPlugin

#Requires special dependencies in a folder next to the PO folder, so config variable
CONFIG(chess) {
       SUBDIRS += ChessPlugin
}
