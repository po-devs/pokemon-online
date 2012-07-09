#!/bin/bash
# This script builds a nice .dmg file for the current PO version.
# http://stackoverflow.com/questions/96882/how-do-i-create-a-nice-looking-dmg-for-mac-os-x-using-command-line-tools

source=/tmp/Pokemon-Online-dmg-creation
title=Pokemon-Online
size=120000

applicationName="Pokemon-Online.app"

finalDMGName="Pokemon-Online.dmg"
if [ -n "$1" ]
then
finalDMGName=$1;
fi

backgroundOrig="mac-deploy-scripts/po-dmg-bg2.png"
backgroundPictureName="po-dmg-bg2.png"

if [ -e $finalDMGName ]
then
  mv $finalDMGName $finalDMGName-backup
fi


if [ -e $source ]
then
  mv $source $source-$(date +%s)
fi

echo "Making dirs"
mkdir $source
cp -r $applicationName $source
echo "Copying background image"
mkdir $source/.background
cp $backgroundOrig $source/.background/

## don't edit

echo "Creating temporary pack"
hdiutil create -srcfolder "${source}" -volname "${title}" -fs HFS+ \
      -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${size}k pack.temp.dmg

device=$(hdiutil attach -readwrite -noverify -noautoopen "pack.temp.dmg" | \
         egrep '^/dev/' | sed 1q | awk '{print $1}')
echo "The name of the device is $device"
ls /Volumes

echo "Fixing folder view options"
echo '
   tell application "Finder"
     tell disk "'${title}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 885, 430}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 72
           set background picture of theViewOptions to file ".background:'${backgroundPictureName}'"
           make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
           set position of item "'${applicationName}'" of container window to {100, 110}
           set position of item "Applications" of container window to {375, 110}
           close
           open
           update without registering applications
           delay 5
           --eject
     end tell
   end tell
' | osascript

echo "Chmodding"
chmod -Rf go-w /Volumes/"${title}"
sync
echo "Detaching the temporary disk image"
hdiutil detach ${device}
sleep 5
echo "Converting to final image"
hdiutil convert pack.temp.dmg -format UDZO -imagekey zlib-level=9 -o "${finalDMGName}"
echo "Removing the temporary disk image"
rm -f pack.temp.dmg 
rm -rf $source
