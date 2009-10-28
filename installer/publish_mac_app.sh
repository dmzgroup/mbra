#!/bin/sh

DEPTH=../../..

CHANNEL=$1

if [ "$CHANNEL" = "" ] ; then
   CHANNEL=devel
fi

VERSION_XML=$DEPTH/bin/macos-opt/MBRA.app/Contents/Resources/config/version.xml
UPDATE=MBRA-`cat $DEPTH/tmp/macos-opt/mbraapp/buildnumber.txt`
INSTALLER=$DEPTH/installers/$UPDATE.dmg

echo "publishing $INSTALLER..."

echo "scp $VERSION_XML dmzdev.org:/home/update/public/latest/macos-$CHANNEL/MBRA.xml"
scp $VERSION_XML dmzdev.org:/home/update/public/latest/macos-$CHANNEL/MBRA.xml

echo "scp ./changelog.html dmzdev.org:/home/update/public/downloads/$UPDATE.html"
scp ./changelog.html dmzdev.org:/home/update/public/downloads/$UPDATE.html

echo "scp $INSTALLER dmzdev.org:/home/update/public/downloads"
scp $INSTALLER dmzdev.org:/home/update/public/downloads

echo "done!"
