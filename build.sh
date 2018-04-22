#!/bin/bash
killall lofoten
args=$(<args.txt)
set -e
./whitgl/scripts/build.sh optimize
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
        game_dir='build/out/Lofoten.app/Contents/MacOS'
else
        game_dir='build/out'
fi
(cd $game_dir; ./lofoten hotreload $args "$@" &)
