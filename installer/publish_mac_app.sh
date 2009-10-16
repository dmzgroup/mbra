#!/bin/sh

DEPTH=../../..

scp $DEPTH/bin/macos-opt/MBRA.app/Contents/Resources/config/version.xml dmzdev.org:/home/update/public/latest/stable/macos/MBRA.xml

scp $DEPTH/installers/MBRA-`cat $DEPTH/tmp/macos-opt/mbraapp/buildnumber.txt`.dmg dmzdev.org:/home/update/public/downloads

