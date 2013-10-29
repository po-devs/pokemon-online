#!/bin/sh

# This script is used to compile PO in an 'install' folder
# Maybe make something to make -j4 optional?

cd ../
rm -Rf install
make clean
rm src/*/Makefile
rm Makefile
qmake CONFIG+="po_clientplugins po_serverplugins release"
make clean
make -j4
mkdir install
cp bin/db install -R
cp bin/trans install -R
cp bin/qml install -R
cp bin/usage_stats install -R
cp bin/Themes install -R
cp scripts/start-* install
cp scripts/install.sh install
cp bin/languages.txt install
cp bin/Pokemon-Online install
cp bin/Server install
cp bin/StatsExtracter install
cp bin/RelayStation install
cp bin/libqtwebsocket.so.1 install
cp bin/libqjson.so.1 install
cp bin/libpo-utilities.so.1 install
cp bin/libpo-pokemoninfo.so.1 install
cp bin/libpo-battlemanager.so.1 install
cp bin/version.ini install

mkdir install/myplugins
mkdir install/serverplugins
cp bin/myplugins/*.so install/myplugins
cp bin/serverplugins/*.so install/serverplugins

rm install/myplugins/*.so.*
rm install/myplugins/*_debug*
rm install/serverplugins/*.so.*
rm install/serverplugins/*_debug*

mv install pokemon-online
