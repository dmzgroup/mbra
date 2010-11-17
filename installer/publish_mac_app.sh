#!/bin/sh

DEPTH=../../..

CHANNEL=$1

if [ "$CHANNEL" = "" ] ; then
   CHANNEL=devel
fi

VERSION_XML=$DEPTH/bin/macos-opt/MBRA.app/Contents/Resources/config/version.xml
UPDATE=MBRA-`cat $DEPTH/tmp/macos-opt/mbraapp/versionnumber.txt`-`cat $DEPTH/tmp/macos-opt/mbraapp/buildnumber.txt`
OLD_UPDATE=MBRA-`cat $DEPTH/tmp/macos-opt/mbraapp/buildnumber.txt`
INSTALLER=$DEPTH/installers/$UPDATE.dmg

echo "publishing $INSTALLER..."

echo "scp $VERSION_XML dmzdev.org:/home/update.dmzdev.org/public/latest/macos-$CHANNEL/MBRA.xml"
scp $VERSION_XML dmzdev.org:/home/update.dmzdev.org/public/latest/macos-$CHANNEL/MBRA.xml

echo "scp ./changelog.html dmzdev.org:/home/update.dmzdev.org/public/downloads/$UPDATE.html"
scp ./changelog.html dmzdev.org:/home/update.dmzdev.org/public/downloads/$UPDATE.html

echo "scp ./changelog.html dmzdev.org:/home/update.dmzdev.org/public/downloads/$OLD_UPDATE.html"
scp ./changelog.html dmzdev.org:/home/update.dmzdev.org/public/downloads/$OLD_UPDATE.html

echo "scp $INSTALLER dmzdev.org:/home/update.dmzdev.org/public/downloads"
scp $INSTALLER dmzdev.org:/home/update.dmzdev.org/public/downloads

echo "ssh dmzdev.org sudo ln -s /home/update.dmzdev.org/public/downloads/$UPDATE.dmg /home/update.dmzdev.org/public/downloads/$OLD_UPDATE.dmg"
ssh dmzdev.org sudo ln -s /home/update.dmzdev.org/public/downloads/$UPDATE.dmg /home/update.dmzdev.org/public/downloads/$OLD_UPDATE.dmg

echo "ssh dmzdev.org sudo chown www-data.admin -R /home/update.dmzdev.org/public"
ssh dmzdev.org sudo chown www-data.admin -R /home/update.dmzdev.org/public

echo "ssh dmzdev.org sudo chmod -R g+w /home/update.dmzdev.org/public"
ssh dmzdev.org sudo chmod -R g+w /home/update.dmzdev.org/public

echo "done!"
