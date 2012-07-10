#!/bin/bash

myScript="$(pwd)/$0"
myDir=$(dirname $myScript)
cd $myDir/..

VERSION=$(grep VERSION ../src/Shared/config.h | sed -n "s/.*\"\(.*\)\".*/\1/p")
final_dmg_name=Pokemon-Online-${VERSION}.dmg
echo Deploying $final_dmg_name

mac-deploy-scripts/bundle_mac_app.sh
mac-deploy-scripts/fix_plugin_linking.py
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion ${VERSION}" Pokemon-Online.app/Contents/Info.plist
mac-deploy-scripts/create-DMG.sh $final_dmg_name
