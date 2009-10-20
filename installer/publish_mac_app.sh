#!/bin/sh

DEPTH=../../..

CHANNEL=$1

if [ "$CHANNEL" = "" ] ; then
   CHANNEL=devel
fi

VERSION_XML=$DEPTH/bin/macos-opt/MBRA.app/Contents/Resources/config/version.xml
INSTALLER=$DEPTH/installers/MBRA-`cat $DEPTH/tmp/macos-opt/mbraapp/buildnumber.txt`.dmg

echo "publishing $INSTALLER..."

echo "scp $VERSION_XML dmzdev.org:/home/update/public/latest/$CHANNEL/macos/MBRA.xml"
scp $VERSION_XML dmzdev.org:/home/update/public/latest/$CHANNEL/macos/MBRA.xml

echo "scp $INSTALLER dmzdev.org:/home/update/public/downloads"
scp $INSTALLER dmzdev.org:/home/update/public/downloads

echo "done!"
