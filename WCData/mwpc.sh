#!/bin/bash
#
# Usage: mwpc.sh <mwpc_file.dat>
#
# If no file is given, an interactive gui pops up
#
# The root macros are kept in $HOME/root_macros
#
# For questions contact Ewa Skup (eskup@fnal.gov)
# or Mickey Chiu (chiu@bnl.gov)
#

export LD_LIBRARY_PATH=${HOME}/root_macros:${LD_LIBRARY_PATH}

datfile=""

if [[ $# -ge 1 ]]
then
  datfile=$1
fi

root.exe ${HOME}/root_macros/mwpc.C\(\"${datfile}\"\)
