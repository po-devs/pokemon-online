#!/bin/bash
cd bin

./test-utilities
if [[ $? != 0 ]]; then
  exit 1 
fi

cp ../tests/data/pokemoninfo/* . -R
./test-pokemoninfo
if [[ $? != 0 ]]; then
  exit 1
fi

cd ..
