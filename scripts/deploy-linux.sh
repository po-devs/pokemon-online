#!/bin/sh

# This script is used to compile PO in an install folder
# Maybe make something to make -j4 optional?

DEST=pokemon-online

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
cp scripts/start-* $DEST
cp scripts/install.sh $DEST
cp bin/languages.txt $DEST
cp bin/Pokemon-Online $DEST
cp bin/Server $DEST
cp bin/BattleServer $DEST
cp bin/StatsExtracter $DEST
cp bin/RelayStation $DEST
cp bin/libqtwebsocket.so.1.0.0 $DEST/libqtwebsocket.so.1
cp bin/libqjson.so.1.0.0 $DEST/libqtjson.so.1
cp bin/libpo-utilities.so.1.0.0 $DEST/libpo-utilities.so.1
cp bin/libpo-pokemoninfo.so.1.0.0 $DEST/libpo-pokemoninfo.so.1
cp bin/libpo-battlemanager.so.1.0.0 $DEST/libpo-battlemanager.so.1
cp bin/libpo-teambuilder.so.1.0.0 $DEST/libpo-teambuilder.so.1
cp bin/version.ini $DEST

echo "Run install.sh to install the dependencies, then run the start-client script or directly the Pokemon-Online binary" > $DEST/README.txt

mkdir $DEST/myplugins
mkdir $DEST/serverplugins
cp bin/myplugins/*.so.1.0.0 $DEST/myplugins
cp bin/serverplugins/*.so.1.0.0 $DEST/serverplugins

rm $DEST/myplugins/*_debug*
rm $DEST/serverplugins/*_debug*

#Use zips instead of folders for sprites
cd $DEST/db/pokes
for g in 1G 2G 3G 4G 5G 6G
do
    cd $g/sprites
    zip -r sprites.zip .
    mv sprites.zip ../
    cd -
    rm -Rf $g/sprites
done
