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
