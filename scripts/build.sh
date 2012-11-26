#!/bin/bash

function showUsageAndExit()
{
    cat <<EOF

Usage:
      build.sh CONFIG TARGET

      CONFIG: debug | release
      TARGET: <empty> | clean 
      (empty means build)
EOF
    exit 1
}

if [ "$1" != "debug" -a "$1" != "release" ]; then
    echo "Error: [$1] is invalid configuration name"
    showUsageAndExit
fi

if [ "$2" != "" -a "$2" != "clean" ]; then
    echo "Error: [$2] is invalid target name"
    showUsageAndExit
fi 

COMPILER_CYGWIN="$COMPILER"
export COMPILER=$(cygpath -aw "$COMPILER_CYGWIN")
if [ "$OSTYPE" == "cygwin" ]; then
    if [ "$2" != "clean" ]; then
        "$(cygpath -a $COMSPEC)" /k build.bat $1 clean
    fi
    "$(cygpath -a $COMSPEC)" /k build.bat $*
else
    if [ "$2" != "clean" ]; then
        "$COMSPEC" //k build.bat $1 clean
    fi
    "$COMSPEC" //k build.bat $*
fi


