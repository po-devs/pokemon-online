#!/bin/bash

myScript="$(pwd)/$0"
myDir=$(dirname $myScript)
cd $myDir/..

final_dmg_name=Pokemon-Online-$(grep VERSION ../src/Shared/config.h | sed -n "s/.*\"\(.*\)\".*/\1/p").dmg
echo Deploying $final_dmg_name

mac-deploy-scripts/bundle_mac_app.sh
mac-deploy-scripts/fix_plugin_linking.py
mac-deploy-scripts/create-DMG.sh $final_dmg_name
