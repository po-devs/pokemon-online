#!/bin/bash
cd bin

function ensure_good_run(){
    if [[ $? != 0 ]]; then
        exit 1 
    fi
}

#Make sure background processes are killed when exiting the script
#We don't want ./Server and ./BattleServer to keep running
trap 'kill -- -$BASHPID' SIGINT SIGTERM EXIT

./test-utilities
ensure_good_run

cp ../tests/data/pokemoninfo/* . -R
./test-pokemoninfo
ensure_good_run

[[ -f BattleServer ]] && (./BattleServer &> /dev/null &) || (./BattleServer_debug &> /dev/null &)

#Give time to the battle server to initialize
sleep 2

./test-battleserver
ensure_good_run

cp ../tests/data/server/* . -R
[[ -f Server ]] && (./Server -H -N &> /dev/null &) || (./Server_debug -H -N &> /dev/null &)

#Give time to the server to initalize
sleep 2

./test-server
ensure_good_run

echo "All tests good!"
cd ..
