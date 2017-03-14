#!/bin/sh
cur_dir=$(pwd)
echo
echo "AMIC test"
echo
echo "test will start in 5 seconds"
echo

sleep 5
alsatest(){
amixer cset numid=17,iface=MIXER,name='ADC Mux'  0 # you can change this parameter #
amixer cset numid=4,iface=MIXER,name='Digital Capture Volume' 20  # you can change this parameter #
amixer cset numid=6,iface=MIXER,name='Mic Volume' 3  # you can change this parameter #

cd $cur_dir

arecord -D hw:0,0 -c 2 -f S16_LE -r 44100 -d 5 Amic_record1.wav # this commad will creat a 10s and 44100Hz Amic_record.wav file #

echo
echo "record has saved successful"
echo
echo "Amic_record1.wav will play"
echo

aplay Amic_record1.wav
}

tinyalsatest(){

cd $cur_dir
tinycap Amic_record2.wav -D 0 -d 0 -c 2 -r 8000 -b 16 -t 5
echo
echo "record has saved successful"
echo
echo "Amic_record2.wav will play"
echo
tinyplay Amic_record2.wav
}

cd
cd /usr/bin
if test -e tinyplay
then tinyalsatest
else alsatest
fi
echo
echo "AMIC test finish"
echo
