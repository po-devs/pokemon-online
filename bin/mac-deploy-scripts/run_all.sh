#!/bin/bash
# Verify correct dir
myScript="$(pwd)/$0"
myDir=$(dirname $myScript)
cd $myDir/..

# Check if package not requested
[ "$1" == "-no-archives" ] && NOPACKAGE=true

# Abort on error
set -e trap

# Almost constants
VERSION=$(grep VERSION ../src/Shared/config.h | sed -n "s/.*\"\(.*\)\".*/\1/p")
ARCHIVE=Pokemon-Online-${VERSION}.zip
DMGARCHIVE=Pokemon-Online-${VERSION}.dmg
APPCAST_URL=http://lamperi.name/pokemon-online/appcast.xml
DOWNLOAD_URL=http://lamperi.name/pokemon-online/$ARCHIVE
RELEASENOTES_URL=http://lamperi.name/pokemon-online/${VERSION}.html

# Update the version of the App
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion ${VERSION}" Pokemon-Online.app/Contents/Info.plist
# Add high-resolution support to the app
/usr/libexec/PlistBuddy -c "Add :NSHighResolutionCapable bool true" Pokemon-Online.app/Contents/Info.plist
# Bundle required stuff into .app
mac-deploy-scripts/bundle_mac_app.sh
# Fix all dylib linkings
mac-deploy-scripts/fix_plugin_linking.py

# Deploy .dmg
if [ -z "$NOPACKAGE" ]
then
    if [ ! -d Pokemon-Online.app/Contents/Frameworks/Sparkle.framework ]
    then
        echo "Error Sparrow.framework not deployed, cannot package!"
        echo "Provide -no-archives switch to this script if you want to just test and not deploy"
        false
    fi
    echo Deploying $DMGARCHIVE
    mac-deploy-scripts/create-DMG.sh $DMGARCHIVE &> /dev/null
fi

# Sign the app
KEYCHAIN_PRIVKEY_NAME="Sign private key"
KEYCHAIN_PUBKEY_NAME="Sign public key"

# Gets the keys from Keychain
# Works on Snow Leopard (verified)
get_keychain_note() {
    security find-generic-password -g -s "$1" 2>&1 1>/dev/null \
      | perl -pe '($_) = /"(.+)"/; s/\\012/\n/g' \
      | perl -MXML::LibXML -e 'print XML::LibXML->new()->parse_file("-")->findvalue(q(//string[preceding-sibling::key[1] = "NOTE"]))'
}

# Only sign if we have the keys in our Keychain
if security find-generic-password -s "$KEYCHAIN_PUBKEY_NAME" &> /dev/null
then
    echo "Copying public key"
    get_keychain_note "$KEYCHAIN_PUBKEY_NAME" > Pokemon-Online.app/Contents/Resources/public_key.pem
    /usr/libexec/PlistBuddy -c "Set :SUPublicDSAKeyFile public_key.pem" Pokemon-Online.app/Contents/Info.plist
    /usr/libexec/PlistBuddy -c "Set :SUFeedURL ${APPCAST_URL}" Pokemon-Online.app/Contents/Info.plist

    if [ -z "$NOPACKAGE" ]
    then
    echo "Compressing"
        ditto -ck --keepParent "Pokemon-Online.app" "${ARCHIVE}"
    
        get_keychain_note "$KEYCHAIN_PRIVKEY_NAME" > /tmp/priv.pem
        SIZE=$(stat -f %z "$ARCHIVE")
        PUBDATE=$(LC_TIME=en_GB date +"%a, %d %b %G %T %z")
        SIGNATURE=$(
    	/usr/bin/openssl dgst -sha1 -binary < "${ARCHIVE}" \
    	| /usr/bin/openssl dgst -dss1 -sign /tmp/priv.pem \
    	| /usr/bin/openssl enc -base64
        )
    
        [ $SIGNATURE ] || { echo Couldn\'t load private keys; false; }
        echo Done signing, add this to appcast.xml file:
        echo
        cat <<EOF
		<item>
			<title>Version $VERSION</title>
			<sparkle:releaseNotesLink>$RELEASENOTES_URL</sparkle:releaseNotesLink>
			<pubDate>$PUBDATE</pubDate>
			<enclosure
				url="$DOWNLOAD_URL"
				sparkle:version="$VERSION"
				type="application/octet-stream"
				length="$SIZE"
				sparkle:dsaSignature="$SIGNATURE"
			/>
		</item>
EOF
    
        echo
        echo Remember to copy the file to $DOWNLOAD_URL
        echo scp $ARCHIVE valssi.fixme.fi:/var/www/lamperi.name/pokemon-online
    fi

fi
