#!/bin/bash
cd bin

./test-utilities
if [[ $? != 0 ]]; then
  exit 1 
fi

#Add further tests with same pattern

cd ..
