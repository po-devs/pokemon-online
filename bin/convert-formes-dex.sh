#!/bin/bash

#Usage: ./convert-formes.sh folder
#Converts all files in the folder and subfolders
#from veekun names to PO names (basically converting
#the forme id in the move files)

#Use this script in combination with veekun_data_extracter and the veekun git repo for the pokedex

folder=$1

function remove() {
    formes=($(seq 650 706) $(seq 712 730) 733)

    len=${#formes[*]}

    #remove the original forme
    i=0
    while [ $i -lt $len ]; do
            forme=${formes[$i]}
            #echo forme: $forme
            find $folder -name "*.txt" -exec bash -c "sed /$forme:0/d '{}' > '{}'~; mv '{}'~ '{}'" \;
            let i++
    done
}

function convert() {
    num=$1
    subnum=$2
    forme=$3

    find $folder -name "*.txt" -exec bash -c "sed s/$forme:0/$num:$subnum/ '{}' > '{}'~; mv '{}'~ '{}'" \;
}

#Rotom
#convert 479 "normal mow heat frost wash fan"
convert 479 1 711
convert 479 2 707
convert 479 3 709
convert 479 4 708
convert 479 5 710
#Kyurem
#convert 646 "normal white black"
convert 646 1 732
convert 646 2 731

remove
