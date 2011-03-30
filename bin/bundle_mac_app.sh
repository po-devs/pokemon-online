#!/bin/bash
#
# A script to pack data files inside .app bundle in Mac
# Run this after compiling Teambuilder
#

myScript="$(pwd)/$0"
myDir=$(dirname $myScript)
[ $(pwd) != $myDir ] && cd $myDir

function bundle_mac_app() {
app=$1
toDir=$app/Contents/Resources
if [ "$1" == "--skip-resources" ] || [ "$1" == "-s" ]
then
	SKIP=1
fi
if [ -z "$SKIP" ]
then

if [ -z "$toDir" ] || [ ! -d "$toDir" ] 
then 
echo Not valid dest dir 
echo Maybe you forgot to compile Teambuilder?
echo Navigate into ../src/Teambuilder
exit 1
fi

[ -d $toDir/Music ] && echo rm -r $toDir/Music && rm -r $toDir/Music
[ -d $toDir/db ] && echo rm -r $toDir/db && rm -r $toDir/db
[ -d $toDir/Themes ] && echo rm -r $toDir/db && rm -r $toDir/Themes

for file in *.txt *.qm Music db database Themes
do
	echo cp -r $file $toDir
	cp -r $file $toDir
done
fi # end of SKIP

fworks=$app/Contents/Frameworks
zlib=libz.1.dylib
libzip=libzip.1.dylib
pokemonlib_long=libpokemonlib.1.0.0.dylib
pokemonlib=libpokemonlib.1.dylib
utilities_long=libutilities.1.0.0.dylib
utilities=libutilities.1.dylib

echo macdeployqt $app
macdeployqt $app -verbose=0

echo Fixing additional libs..
if [ -e $zlib ]
then
    echo zlib..
    cp $zlib $fworks
    install_name_tool -id @executable_path/../Frameworks/$zlib $fworks/$zlib
fi
if [ -e "$libzip" ]
then
    echo libzip..
    cp "$libzip" "$fworks"
    install_name_tool -id @executable_path/../Frameworks/"$libzip" $fworks/"$libzip"
    path=$(otool -L "$libzip" | grep "/$zlib" | awk '{print $1}')
    if [ -n "$path" -a ! "$path" == "/usr/lib/libz.1.dylib" ]; then
        install_name_tool -change "$path" @executable_path/../Frameworks/"$zlib" $fworks/"$libzip"
    fi
else
    echo "Warning! No libzip found, application may not be functional!"
fi

echo pokemonlib..
if [ ! -e $pokemonlib_long ]; then
    echo "Error! No pokemonlib found, please compile it first!"
    exit 1
fi
cp $pokemonlib_long $fworks/
ln -sf $fworks/$pokemonlib_long $pokemonlib
install_name_tool -id @executable_path/../Frameworks/$pokemonlib $pokemonlib
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $pokemonlib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $pokemonlib
install_name_tool -change QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml $pokemonlib
install_name_tool -change $utilities @executable_path/../Frameworks/$utilities $pokemonlib

echo utilities..
if [ ! -e $utilities_long ]; then
    echo "Error! No libutilities found, please compile it first!"
    exit 1
fi
cp $utilities_long $fworks
ln -sf $fworks/$utilities_long $utilities
install_name_tool -id @executable_path/../Frameworks/$utilities $utilities
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $utilities
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $utilities

if [ $app == Server.app -a -e serverplugins ]
then
    for plugin in serverplugins/*.1.0.0.dylib
    do
        echo $plugin
        pluginbase=$(basename $plugin)
        install_name_tool -id @executable_path/../Frameworks/$pluginbase $plugin
        install_name_tool -change $utilities @executable_path/../Frameworks/$utilities $plugin
        install_name_tool -change $pokemonlib @executable_path/../Frameworks/$pokemonlib $plugin
        install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $plugin
        install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $plugin
    done
fi

echo Done
}

if [ -d Pokemon-Online.app ]
then
    bundle_mac_app Pokemon-Online.app
fi
if [ -d Server.app ]
then
    bundle_mac_app Server.app
fi
