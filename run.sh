#!/bin/sh

. ../scripts/envsetup.sh

export MBRA_WORKING_DIR="."
export MBRA_RESOURCE="../../mbra/mbra.rcc"

$RUN_DEBUG$BIN_HOME/mbra $*
