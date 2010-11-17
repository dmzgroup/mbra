#!/bin/sh
DEPTH=../../..

CHANNEL=$1

if [ "$CHANNEL" = "" ] ; then
   CHANNEL=devel
fi

VERSION_XML=$DEPTH/bin/win32-opt/MBRA.app/config/version.xml
UPDATE=MBRA-`cat $DEPTH/tmp/win32-opt/mbraapp/versionnumber.txt`-`cat $DEPTH/tmp/win32-opt/mbraapp/buildnumber.txt`
OLD_UPDATE=MBRA-`cat $DEPTH/tmp/win32-opt/mbraapp/buildnumber.txt`
INSTALLER=$DEPTH/installers/$UPDATE.exe

echo "publishing $INSTALLER..."

echo "scp $VERSION_XML dmzdev.org:/home/update.dmzdev.org/public/latest/win32-$CHANNEL/MBRA.xml"
scp $VERSION_XML dmzdev.org:/home/update.dmzdev.org/public/latest/win32-$CHANNEL/MBRA.xml

echo "scp ./changelog.html dmzdev.org:/home/update.dmzdev.org/public/downloads/$UPDATE.html"
scp ./changelog.html dmzdev.org:/home/update.dmzdev.org/public/downloads/$UPDATE.html

echo "scp $INSTALLER dmzdev.org:/home/update.dmzdev.org/public/downloads"
scp $INSTALLER dmzdev.org:/home/update.dmzdev.org/public/downloads

echo "ssh dmzdev.org sudo ln -s /home/update.dmzdev.org/public/downloads/$UPDATE.exe /home/update.dmzdev.org/public/downloads/$OLD_UPDATE.exe"
ssh dmzdev.org sudo ln -s /home/update.dmzdev.org/public/downloads/$UPDATE.exe /home/update.dmzdev.org/public/downloads/$OLD_UPDATE.exe

echo "ssh dmzdev.org sudo chown www-data.admin -R /home/update.dmzdev.org/public"
ssh dmzdev.org sudo chown www-data.admin -R /home/update.dmzdev.org/public

echo "ssh dmzdev.org sudo chmod -R g+w /home/update.dmzdev.org/public"
ssh dmzdev.org sudo chmod -R g+w /home/update.dmzdev.org/public

echo "done!"
