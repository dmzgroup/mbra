#!/bin/sh
DEPTH=../../..
cp $DEPTH/bin/win32-opt/MBRA.app/config/version.xml $DEPTH/swupdate/dmz/latest/stable/win32/MBRA.xml
cp $DEPTH/MBRA-`cat $DEPTH/tmp/win32-opt/mbraapp/buildnumber.txt`.exe $DEPTH/swupdate/dmz/downloads
$DEPTH/swupdate/google_appengine/appcfg.py update $DEPTH/swupdate/dmz
