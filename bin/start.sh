#!/bin/sh
POPATH=~/code/pokemon/bin
export LD_LIBRARY_PATH="$POPATH:$LD_LIBRARY_PATH"
cd "$POPATH"
exec ./Pokemon-Online
