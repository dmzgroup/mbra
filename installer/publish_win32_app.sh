#!/bin/sh
DEPTH=../../..

CHANNEL=$1

if [ "$CHANNEL" = "" ] ; then
   CHANNEL=devel
fi

VERSION_XML=$DEPTH/bin/win32-opt/MBRA.app/config/version.xml
INSTALLER=$DEPTH/installers/MBRA-`cat $DEPTH/tmp/win32-opt/mbraapp/buildnumber.txt`.exe

echo "publishing $INSTALLER..."

echo "scp $VERSION_XML dmzdev.org:/home/update/public/latest/$CHANNEL/win32/MBRA.xml"
scp $VERSION_XML dmzdev.org:/home/update/public/latest/$CHANNEL/win32/MBRA.xml

echo "scp $INSTALLER dmzdev.org:/home/update/public/downloads"
scp $INSTALLER dmzdev.org:/home/update/public/downloads

echo "done!"