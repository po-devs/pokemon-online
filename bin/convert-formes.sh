#!/bin/bash

#Usage: ./convert-formes.sh folder
#Converts all files in the folder and subfolders
#from veekun names to PO names (basically replacing
#the forme text id to its numerical id)

#Use this script on the packages located here: http://veekun.com/dex/downloads!

folder=$1

function convert() {
    num=$1
    formes=($2)

    len=${#formes[*]}

    #remove the original forme
    find $folder -name "*$num-${formes[0]}.png" -exec rm {} \;
    i=1
    while [ $i -lt $len ]; do
            forme=${formes[$i]}
            #echo forme: $forme
            find $folder -name "*$num-$forme.png" -exec bash -c 'mv {} $(echo {} | sed s/'$num'-'$forme'.png/'$num'-'$i'.png/)' \;
            let i++
    done
}

#Pichu
convert 172 "normal spiky-eared"
#Unown
convert 201 'a b c d e f g h i j k l m n o p q r s t u v w x y z exclamation question'
#Castform
convert 351 "normal snowy rainy sandy sunny"
#Deoxys
convert 386 "normal attack defense speed"
#Burmy, Wormadam
convert 412 "plant sandy trash"
convert 413 "plant sandy trash"
#Cherrim
convert 421 "overcast sunshine"
#Shellos, Gastrodon
convert 422 "west east"
convert 423 "west east"
#Rotom
convert 479 "normal mow heat frost wash fan"
#Giratina
convert 487 "altered origin"
#Shaymin
convert 492 "land sky"
#Arceus
convert 493 "normal fighting flying poison ground rock bug ghost steel fire water grass electric psychic ice dragon dark"
#Basculin
convert 550 "red-striped blue-striped"
#Darmanitan
convert 555 "standard zen"
#Deerling, Sawsbuck
convert 585 "spring summer autumn winter"
convert 586 "spring summer autumn winter"
#Tornadus
convert 641 "incarnate therian"
#Thundurus
convert 642 "incarnate therian"
#Landorus
convert 645 "incarnate therian"
#Kyurem
convert 646 "normal white black"
#Keldeo
convert 647 "regular resolution"
#Meloetta
convert 648 "aria pirouette"
#Genesect
convert 649 "normal douse shock burn chill"
