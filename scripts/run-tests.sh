#!/bin/bash
cd bin

function ensure_good_run(){
    if [[ $? != 0 ]]; then
        exit 1 
    fi
}

./test-utilities
ensure_good_run

cp ../tests/data/pokemoninfo/* . -R
./test-pokemoninfo
ensure_good_run

pkill BattleServer
[[ -f BattleServer ]] && (./BattleServer &> /dev/null &) || (./BattleServer_debug &> /dev/null &)

#Give time to the battle server to initialize
sleep 2

./test-battleserver
ensure_good_run

echo "All tests good!"
cd ..
