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

   return
   # TODO: move shader plugins into .pro files 
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
