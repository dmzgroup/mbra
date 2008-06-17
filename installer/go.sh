#!/bin/sh
DEPTH=../../..
lmk -m opt -b
cp -r $DEPTH/bin/macos-opt/MBRA.app $DEPTH
mkdir $DEPTH/MBRA.app/Contents/Frameworks/Qt
cp $DEPTH/depend/Qt/QtCore $DEPTH/MBRA.app/Contents/Frameworks/Qt
cp $DEPTH/depend/Qt/QtGui $DEPTH/MBRA.app/Contents/Frameworks/Qt
cp $DEPTH/depend/Qt/QtXml $DEPTH/MBRA.app/Contents/Frameworks/Qt
cp $DEPTH/depend/Qt/QtSvg $DEPTH/MBRA.app/Contents/Frameworks/Qt
cp $DEPTH/depend/Qt/QtOpenGL $DEPTH/MBRA.app/Contents/Frameworks/Qt
hdiutil create -srcfolder $DEPTH/MBRA.app $DEPTH/MBRA-`cat $DEPTH/tmp/macos-opt/mbraapp/buildnumber.txt`.dmg
hdiutil internet-enable -yes -verbose $DEPTH/MBRA-`cat $DEPTH/tmp/macos-opt/mbraapp/buildnumber.txt`.dmg
rm -rf $DEPTH/MBRA.app/
