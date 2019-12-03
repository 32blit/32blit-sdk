#!/bin/bash

FILENAME=$1
FILEPATH=$(pwd | sed 's/\//\\/g' | sed 's/\\mnt\\c/C:/')\\build\\

if [ "$FILENAME" == "" ]; then
    echo "Usage: $0 <project-name>"
    exit 1
fi

../tools/dfu --verbose build --force --out build/$FILENAME.dfu build/$FILENAME.bin
cmd.exe /c wsl-flash.bat $FILEPATH$FILENAME