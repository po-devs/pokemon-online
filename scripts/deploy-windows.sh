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

DEST=pokemon-online
alias make=mingw32-make

cd ../
rm -Rf $DEST
make clean
rm -Rf build/release
rm src/*/Makefile
rm src/*/*/Makefile
rm Makefile
qmake CONFIG+="po_clientplugins po_serverplugins release"
make clean
make -j4
ensure_good_run
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
rm $DEST/*_debug.exe
rm $DEST/*_debug.dll
cp bin/version.ini $DEST

mkdir $DEST/myplugins
mkdir $DEST/serverplugins
cp bin/myplugins/*.dll $DEST/myplugins
cp bin/serverplugins/*.dll $DEST/serverplugins
#cp lib/windows/*.dll $DEST
rm $DEST/myplugins/*_debug.dll
rm $DEST/serverplugins/*_debug.dll

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

