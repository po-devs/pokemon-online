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
         mkdir -p $toDir
       fi

       [ -d $toDir/Music ] && echo rm -r $toDir/Music && rm -r $toDir/Music
       [ -d $toDir/db ] && echo rm -r $toDir/db && rm -r $toDir/db
       [ -d $toDir/Themes ] && echo rm -r $toDir/db && rm -r $toDir/Themes

       for file in languages.txt trans Music db database Themes qml
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
   battlelib_long=libbattlelib.1.0.0.dylib
   battlelib=libbattlelib.1.dylib
   
   qt_prefix=/Users/lamperi/QtSDK/Desktop/Qt/474/gcc/lib/

   mkdir -p $fworks

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
       path=$(otool -L "$fworks/$libzip" | grep "/$zlib" | awk '{print $1}')
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
   cp $pokemonlib_long $fworks/$pokemonlib_long
   ln -sf $pokemonlib_long $fworks/$pokemonlib
   install_name_tool -id @executable_path/../Frameworks/$pokemonlib $fworks/$pokemonlib
   install_name_tool -change ${qt_prefix}QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $fworks/$pokemonlib
   install_name_tool -change ${qt_prefix}QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $fworks/$pokemonlib
   install_name_tool -change ${qt_prefix}QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml $fworks/$pokemonlib
   install_name_tool -change $utilities @executable_path/../Frameworks/$utilities $fworks/$pokemonlib
   path=$(otool -L $fworks/$pokemonlib | grep "/$libzip" | awk '{print $1}')
   if [ -n "$path" ]; then
      install_name_tool -change $path @executable_path/../Frameworks/$libzip $fworks/$pokemonlib
   else
      echo "Error! Couldn't link to static libzip!"
   fi

   echo utilities..
   if [ ! -e $utilities_long ]; then
       echo "Error! No libutilities found, please compile it first!"
       exit 1
   fi
   cp $utilities_long $fworks/$utilities_long
   ln -sf $utilities_long $fworks/$utilities
   install_name_tool -id @executable_path/../Frameworks/$utilities $fworks/$utilities
   install_name_tool -change ${qt_prefix}QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $fworks/$utilities
   install_name_tool -change ${qt_prefix}QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $fworks/$utilities

   echo battle lib..
   if [ ! -e $battlelib_long ]; then
       echo "Error! No libbattlelib found, please compile it first!"
       exit 1
   fi
   cp $battlelib_long $fworks/$battlelib_long
   ln -sf $battlelib_long $fworks/$battlelib
   install_name_tool -id @executable_path/../Frameworks/$battlelib $fworks/$battlelib
   install_name_tool -change ${qt_prefix}QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $fworks/$battlelib
   install_name_tool -change ${qt_prefix}QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $fworks/$battlelib
   install_name_tool -change ${qt_prefix}QtOpenGL.framework/Versions/4/QtOpenGL @executable_path/../Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL $fworks/$battlelib
   install_name_tool -change ${qt_prefix}QtDeclarative.framework/Versions/4/QtDeclarative @executable_path/../Frameworks/QtDeclarative.framework/Versions/4/QtDeclarative $fworks/$battlelib
   install_name_tool -change $utilities @executable_path/../Frameworks/$utilities $fworks/$battlelib
   install_name_tool -change $pokemonlib @executable_path/../Frameworks/$pokemonlib $fworks/$battlelib

   shadersplugin=Pokemon-Online.app/Contents/imports/Qt/labs/shaders/libqmlshadersplugin.dylib
   install_name_tool -id @executable_path/../imports/Qt/labs/shaders/libqmlshadersplugin.dylib $shadersplugin
   install_name_tool -change ${qt_prefix}QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $shadersplugin
   install_name_tool -change ${qt_prefix}QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $shadersplugin
   install_name_tool -change ${qt_prefix}QtOpenGL.framework/Versions/4/QtOpenGL @executable_path/../Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL $shadersplugin
   install_name_tool -change ${qt_prefix}QtDeclarative.framework/Versions/4/QtDeclarative @executable_path/../Frameworks/QtDeclarative.framework/Versions/4/QtDeclarative $shadersplugin
   particleplugin=Pokemon-Online.app/Contents/imports/Qt/labs/particles/libqmlparticlesplugin.dylib
   install_name_tool -id @executable_path/../imports/Qt/labs/shaders/libqmlparticlesplugin.dylib $particleplugin
   install_name_tool -change ${qt_prefix}QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $particleplugin
   install_name_tool -change ${qt_prefix}QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $particleplugin
   install_name_tool -change ${qt_prefix}QtOpenGL.framework/Versions/4/QtOpenGL @executable_path/../Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL $particleplugin
   install_name_tool -change ${qt_prefix}QtDeclarative.framework/Versions/4/QtDeclarative @executable_path/../Frameworks/QtDeclarative.framework/Versions/4/QtDeclarative $particleplugin
   install_name_tool -change ${qt_prefix}QtSvg.framework/Versions/4/QtSvg @executable_path/../Frameworks/QtSvg.framework/Versions/4/QtSvg $particleplugin
   install_name_tool -change ${qt_prefix}QtScript.framework/Versions/4/QtScript @executable_path/../Frameworks/QtScript.framework/Versions/4/QtScript $particleplugin

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
    rm Server.app/Contents/Resources/members.txt
    touch Server.app/Contents/Resources/members.txt
fi
