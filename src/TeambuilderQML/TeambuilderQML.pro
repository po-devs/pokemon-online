TEMPLATE = app
QT += gui quick

TARGET = Pokemon-Online
CONFIG += c++11

SOURCES =   main.cpp \
            serverchoicemodel.cpp \
            ../Teambuilder/analyze.cpp

HEADERS =   serverchoicemodel.h \
            ../Teambuilder/analyze.h

INCLUDEPATH = "../libraries"

RESOURCES += \
    qml.qrc

include(../Shared/Common.pri)

LIBS += $$teambuilder
