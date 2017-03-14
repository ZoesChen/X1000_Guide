#!/bin/sh
cur_dir=$(pwd)

echo
echo "DMIC test"
echo
echo "test will start in 5 seconds"
echo
sleep 5
alsatest(){

cd $cur_dir

arecord -D hw:0,2 -c 2 -f S16_LE -r 8000 -d 5 Dmic_record1.wav 

echo
echo "record has saved successful"
echo
echo "Dmic_record1.wav will play"
echo

aplay Dmic_record1.wav
}

tinyalsatest(){

cd $cur_dir
tinycap Dmic_record2.wav -D 0 -d 2 -c 2 -r 8000 -b 16 -t 5
echo
echo "record has saved successful"
echo
echo "Dmic_record2.wav will play"
echo
tinyplay Dmic_record2.wav
}

cd
cd /usr/bin
if test -e tinyplay
then tinyalsatest
else alsatest
fi
echo
echo "DMIC test finish"
echo
