#!/bin/sh
cur_dir=$(pwd)
echo
echo "Tfcard read & write test"
echo
cd

if test -t /mnt/sd
then
	echo
	echo "sd dir has been created"
else
	echo "sd dir will be created"
	mkdir /mnt/sd
fi

cd $cur_dir

if test -e Dmic_record1.wav
then
	testfile=$cur_dir/Dmic_record1.wav
else
	if test -e Dmic_record2.wav
		then testfile=$cur_dir/Dmic_record2.wav
		else

		if test -e Amic_record1.wav
		then
			testfile=$cur_dir/Amic_record1.wav
		else
			if test -e Amic_record2.wav
			then testfile=$cur_dir/Amic_record2.wav
			else
			echo "please run DMICDemo.sh or AMICDemo.sh"
			fi
		fi
	fi
fi
mount -t vfat /dev/mmcblk0p1 /mnt/sd


echo "TF card files:"
cd
cd /mnt/sd
ls
sleep 2

cp $testfile /mnt/sd/test.wav
sync
diff $testfile /mnt/sd/test.wav
if [ $? -ne 0 ];then
	echo
	echo "Tfcard read failed"
	echo
else
	echo
	echo "Tfcard read pass"
	echo
fi

cp /mnt/sd/test.wav /mnt/sd/testtest.wav
sync
diff /mnt/sd/test.wav /mnt/sd/testtest.wav
if [ $? -ne 0 ];then
	echo
	echo "Tfcard write failed"
	echo
else
	echo
	echo "Tfcard write pass"
	echo
fi

echo
echo "Tfcard test finish"
echo
echo "Tf card files:"
cd /mnt/sd
ls
cd

