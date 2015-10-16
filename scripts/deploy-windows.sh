#!/bin/bash

# This script is used to compile PO in an install folder
# Maybe make something to make -j4 optional?

function zip_files(){
    cd $1
    name=`basename $1 .txt`
    zip -r $name.zip .
    mv $name.zip ../
    cd -
    rm -Rf $1
}
function ensure_good_run(){
    if [[ $? != 0 ]]; then
        exit 1 
    fi
}

export PATH=$PATH;C:/Qt/Qt5.0.2/5.0.2/mingw47_32/bin;C:/Qt/Qt5.0.2/Tools/MinGW/bin;
DEST=pokemon-online
MAKE=mingw32-make

cd ../
rm -Rf $DEST
$MAKE clean
rm -Rf build/release
rm src/*/Makefile
rm src/*/*/Makefile
rm Makefile
qmake CONFIG+="po_clientplugins po_serverplugins release"
$MAKE clean
$MAKE -j4
ensure_good_run
mkdir $DEST
cp pokemon-template/* $DEST -R
cp bin/db $DEST -R
cp bin/trans $DEST -R
cp bin/qml $DEST -R
cp bin/usage_stats $DEST -R
cp bin/Themes $DEST -R
cp bin/languages.txt $DEST
cp bin/*.exe $DEST
cp bin/*.dll $DEST
rm $DEST/*_debug.exe
rm $DEST/test-*.exe
rm $DEST/*_debug.dll
cp bin/version.ini $DEST

mkdir $DEST/clientplugins
mkdir $DEST/serverplugins
mkdir $DEST/battleserverplugins
cp bin/clientplugins/*.dll $DEST/clientplugins
cp bin/serverplugins/*.dll $DEST/serverplugins
cp bin/battleserverplugins/*.dll $DEST/battleserverplugins
#cp lib/windows/*.dll $DEST
rm $DEST/clientplugins/*_debug.dll
rm $DEST/serverplugins/*_debug.dll
rm $DEST/battleserverplugins/*_debug.dll

#Use zips instead of folders for sprites
cd $DEST/db/pokes
for g in 1G 2G 3G 4G 5G 6G
do
    zip_files $g/sprites
done
zip_files icons
zip_files cries
cd ..
zip_files items/items
zip_files items/berries
cd ..

