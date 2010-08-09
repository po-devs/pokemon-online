#!/bin/bash
#
# A script to pack data files inside .app bundle in Mac
# Run this after compiling Teambuilder
#

myScript="$(pwd)/$0"
myDir=$(dirname $myScript)
[ $(pwd) != $myDir ] && cd $myDir

toDir=Pokemon-Online.app/Contents/Resources

if [ -z "$toDir" ] || [ ! -d "$toDir" ] 
then 
echo Not valid dest dir 
echo Maybe you forgot to compile Teambuilder?
echo Navigate into ../src/Teambuilder
exit 1
fi

[ -d $toDir/Music ] && echo rm -r $toDir/Music && rm -r $toDir/Music
[ -d $toDir/db ] && echo rm -r $toDir/db && rm -r $toDir/db

for file in *.ttf *.txt *.qm Music db 
do
	echo cp -r $file $toDir
	cp -r $file $toDir
done

echo macdeployqt Pokemon-Online.app
macdeployqt Pokemon-Online.app

echo Fixing additional libs
cp libpokemonlib.1.0.0.dylib Pokemon-Online.app/Contents/Frameworks
cp libutilities.1.0.0.dylib Pokemon-Online.app/Contents/Frameworks
# change this if your libzip is located elsewhere
cp /usr/local/lib/libzip.1.dylib Pokemon-Online.app/Contents/Frameworks

cd Pokemon-Online.app/Contents/Frameworks
ln -s libpokemonlib.1.0.0.dylib libpokemonlib.1.dylib
ln -s libutilities.1.0.0.dylib libutilities.1.dylib

install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore libpokemonlib.1.0.0.dylib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui libpokemonlib.1.0.0.dylib
install_name_tool -change QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml libpokemonlib.1.0.0.dylib
install_name_tool -change libutilities.1.dylib @executable_path/../Frameworks/libutilities.1.dylib libpokemonlib.1.dylib
install_name_tool -change /usr/local/lib/libzip.1.dylib @executable_path/../Frameworks/libzip.1.dylib libpokemonlib.1.0.0.dylib

install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore libutilities.1.0.0.dylib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui libutilities.1.0.0.dylib

echo Done
