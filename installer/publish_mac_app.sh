#!/bin/sh
DEPTH=../../..
cp $DEPTH/bin/macos-opt/MBRA.app/Contents/Resources/config/version.xml $DEPTH/swupdate/dmz/latest/stable/macos/MBRA.xml
cp $DEPTH/MBRA-`cat $DEPTH/tmp/macos-opt/mbraapp/buildnumber.txt`.dmg $DEPTH/swupdate/dmz/downloads
$DEPTH/swupdate/google_appengine/appcfg.py update $DEPTH/swupdate/dmz
