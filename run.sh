#!/bin/sh

export BIN_TYPE=$1

if [ "$BIN_TYPE" = "" ] ; then
   export BIN_TYPE=debug
fi

if [ `uname` = "Darwin" ] ; then
   export BIN_HOME=../../bin/macos-$BIN_TYPE ;
elif [ `uname` = "Linux" ] ; then
   export BIN_HOME=../../bin/linux-$BIN_TYPE ;
   export LD_LIBRARY_PATH=$BIN_HOME ;
elif [ `uname -o` = "Cygwin" ] ; then
   export BIN_HOME=../../bin/win32-$BIN_TYPE ;
else
   echo "Unsupported platform: " `uname`
   exit -1 ;
fi

export MBRA_WORKING_DIR="."
export MBRA_RESOURCE="../../mbra/mbra.rcc"
#export MBRA_STYLE_SHEET="../../mbra/qss/mbra.qss"

$BIN_HOME/mbra $2
