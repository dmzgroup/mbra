#!/bin/sh

. ../scripts/envsetup.sh
export MBRA_WORKING_DIR="./"
$RUN_DEBUG$BIN_HOME/mbra $*
