#!/bin/sh

cd $1

rename 's/(\d+)-(\d+)/$1_$2/' *.png
rename 's/(\d+)-(\d+)/$1_$2/' */*.png
rename 's/(\d+)-(\d+)/$1_$2/' */*/*.png
rename 's/(\d+)-(\d+)/$1_$2/' */*/*/*.png

rename 's/(.+)\.png/p$1_front.png/' *.png
cd female
rename 's/(.+)\.png/p$1_frontf.png/' *.png
cd ../shiny
rename 's/(.+)\.png/p$1_fronts.png/' *.png
cd female
rename 's/(.+)\.png/p$1_frontfs.png/' *.png
cd ../../

cd back
rename 's/(.+)\.png/p$1_back.png/' *.png
cd female
rename 's/(.+)\.png/p$1_backf.png/' *.png
cd ../shiny
rename 's/(.+)\.png/p$1_backs.png/' *.png
cd female
rename 's/(.+)\.png/p$1_backfs.png/' *.png
cd ../../../

mv */*.png .
mv */*/*.png .
mv */*/*/*.png .

mogrify -trim *.png
