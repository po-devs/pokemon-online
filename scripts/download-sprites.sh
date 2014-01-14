#! /bin/sh

#Usage: ./convert-formes.sh
#Converts all files in the folder and subfolders
#from veekun names to PO names (basically converting
#the forme id in the move files)

#Use this script in combination with veekun_data_extracter and the veekun git repo for the pokedex

mkdir back
mkdir back/shiny
mkdir shiny

len=650

i=1
while [ $i -lt $len ]; do
    val=`printf "%03d" $i`
    echo i: $i
    wget http://sprites.pokecheck.org/i/$val.gif
    wget http://sprites.pokecheck.org/b/$val.gif -P back
    wget http://sprites.pokecheck.org/s/$val.gif -P shiny
    wget http://sprites.pokecheck.org/bs/$val.gif -P back/shiny

    i=$(($i+1))
done
