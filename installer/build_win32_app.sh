#!/bin/sh
DEPTH=../../..
rm -f ./mbrasetup.exe
lmk -m opt -b
$DEPTH/depend/InnoSetup5/ISCC.exe mbra.iss
INSTALLER_PATH=$DEPTH/installers
if [ ! -d $INSTALLER_PATH ] ; then
   mkdir $INSTALLER_PATH
fi
cp mbrasetup.exe $INSTALLER_PATH/MBRA-`cat $DEPTH/tmp/win32-opt/mbraapp/versionnumber.txt`-`cat $DEPTH/tmp/win32-opt/mbraapp/buildnumber.txt`.exe
