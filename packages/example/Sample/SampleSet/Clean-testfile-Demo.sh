#!/bin/sh
cur_dir=$(pwd)
echo
echo "this program will clean test record"
echo

cd
cd mnt
if test -t /mnt/sd
then sleep 1
else mkdir sd
fi
cd
mount -t vfat /dev/mmcblk0p1 /mnt/sd

sleep 2
echo "delete test files in Tfcard"
cd
cd mnt/sd
echo "Tf card files: "
sleep 2
ls

if test -e test.wav
then rm test.wav
else
	echo "there is no test files in Tfcard"
fi

if test -e testtest.wav
then rm testtest.wav
else

	echo "there is no testtest.wav in Tfcard"
fi
echo "Tf card files:"
ls
sleep 2

cd $cur_dir

echo
echo "delete Amic record file"
if test -e Amic_record1.wav
then rm Amic_record1.wav
else if test -e Amic_record2.wav
	then rm Amic_record2.wav
	else
	echo
	echo "there is no Amic record file"
	fi
fi


echo
echo "delete Dmic record file"
if test -e Dmic_record1.wav
then rm Dmic_record1.wav
else if test -e Dmic_record2.wav
	then rm Dmic_record2.wav
	else
	echo
	echo "there is no Dmic record file"
	fi
fi

cd

echo
echo "umount Tfcard"
umount /dev/mmcblk0p1
if test -e /mnt/sd
then rm -r /mnt/sd
	 echo
	 echo "sd dir has been delete"
else
	echo
	echo "there is no sd dir"
fi

echo
echo "delete photos"
cd
cd testsuite
if test -e ingenic_mmap.jpg
then rm ingenic_mmap.jpg
else echo
	echo "check another module photo"
fi
if test -e ingenic_userptr.jpg
then rm ingenic_userptr.jpg
else echo

	echo "there is no photo by camera"
fi

echo
echo "all test files have been clean"
echo
