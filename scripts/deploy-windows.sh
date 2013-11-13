#!/bin/sh

# This script is used to compile PO in an install folder
# Maybe make something to make -j4 optional?

DEST=pokemon-online
alias make=mingw32-make

cd ../
rm -Rf $DEST
make clean
rm src/*/Makefile
rm Makefile
qmake CONFIG+="po_clientplugins po_serverplugins release"
make clean
make -j4
mkdir $DEST
cp bin/db $DEST -R
cp bin/trans $DEST -R
cp bin/qml $DEST -R
cp bin/usage_stats $DEST -R
cp bin/Themes $DEST -R
cp bin/languages.txt $DEST
cp bin/Pokemon-Online.exe $DEST
cp bin/*.exe $DEST
cp bin/*.dll $DEST
rm $DEST/*d.exe
rm $DEST/*d.dll
cp bin/version.ini $DEST

mkdir $DEST/myplugins
mkdir $DEST/serverplugins
cp bin/myplugins/*.dll $DEST/myplugins
cp bin/serverplugins/*.dll $DEST/serverplugins
#cp lib/windows/*.dll $DEST
rm $DEST/myplugins/*d.dll
rm $DEST/serverplugins/*d.dll
