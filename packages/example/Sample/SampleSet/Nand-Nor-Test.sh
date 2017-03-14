#!/bin/bash

echo "**********************************************"
echo "* * Please input which module you want test ."
echo "**********************************************"
echo "* Enter  1 : copy file form nand to sd "
echo "* Enter  2 : copy file form nand to nand "
echo "* Enter  3 : copy file form sd to sd "
echo "* Enter  4 : copy file form sd to nand "
echo
read copy_module
case $copy_module in
	1)  echo 'You select 1, nand -> sd'
		echo
		;;
	2)  echo 'You select 2, nand -> nand'
		echo
		;;
	3)  echo 'You select 3, sd -> sd'
		echo
		;;
	4)  echo 'You select 4, sd ->nand'
		echo
		;;
	*) echo 'choice err ,exit!!'
		exit
		;;
esac
echo "***************************************************"
echo "* * Please choice which Size you want copy ."
echo "***************************************************"
echo "* Enter  1 : auto random Size "
echo "* Enter  2 : setting a fix Size "
echo
read test_module
case $test_module in
	1)  echo 'You select 1, RANDOM_SIZE'
		echo
		;;
	2)  echo 'You select 2, Fix SIZE'
		echo
		;;
	*)  echo 'choice err ,exit!!'
		exit
		;;
esac

CNT=0
MINISIZE=1
DATA_FREECAP=`busybox df | busybox awk '{if($6=="/") print $4}'`

function GetRandomSize()
{
	DATA_FREECAP=`busybox df | busybox awk '{if($6=="/") print $4}'`
	DATA_FREECAP=`busybox expr $DATA_FREECAP \* 1024`
	BIGFILE_LIMIT=`busybox expr $DATA_FREECAP \/ 5`
	RandomNum=$(cat /dev/urandom | head -n 10 | cksum | awk -F ' ' '{print $1}')
	#echo "=======Ra Rnum=$RandomNum BIGFILE_LIMIT=$BIGFILE_LIMIT"
	RANDOM_SIZE=$(($RandomNum%$BIGFILE_LIMIT+$MINISIZE))
	#echo "---->>>> $RANDOM_SIZE"
	RANDOM_SIZE_UN=${RANDOM_SIZE#-}
	#echo "---->>>>RS $RANDOM_SIZE_UN"
	#echo "NAND_COPY_TEST: /data free capcity: $DATA_FREECAP"
	#echo "NAND_COPY_TEST: big file size reference: $BIGFILE_LIMIT"
	echo "NAND_COPY_TEST: random size: $RANDOM_SIZE_UN"
	return $RANDOM_SIZE_UN
}


function GetBigfile()
{
	busybox dd if=/dev/urandom of=/bigfile bs=$1 count=1 > /dev/null 2>&1
	sync
	if [ -f /bigfile ]; then
		FILESIZE=`ls -l /bigfile | busybox awk '{print $5}'`
		if [ $FILESIZE -eq 0 ]; then
			echo "NAND_COPY_TEST: ERROR: /bigfile size is 0, exit::::$CNT" > /dev/console
			exit
		fi
	else
		echo "NAND_COPY_TEST: error, can't creat /bigfile, exit::::$CNT" > /dev/console
		exit
	fi
}

function CopyBigfile()
{
	case $1 in
		1)	cp /bigfile /mnt/sd/bigfile-back
			;;
		2)	cp /bigfile /bigfile-back
			;;
		3)	if [ -f /bigfile ];then
				cp /bigfile /mnt/sd/bigfile
				rm /bigfile
			else
				cp /mnt/sd/bigfile /mnt/sd/bigfile-back
			fi
			;;
		4)	if [ -f /bigfile ];then
				cp /bigfile /mnt/sd/bigfile
				rm /bigfile
			else
				cp /mnt/sd/bigfile /bigfile-back
			fi
			;;
		*) echo 'choice err ,exit!!'
			exit
			;;
	esac
}

function DiffFile()
{
	case $1 in
		1)  echo 'You select 1, nand -> sd'
			diff /bigfile /mnt/sd/bigfile-back
			;;
		2)  echo 'You select 2, nand -> nand'
			diff /bigfile /bigfile-back
			;;
		3)  echo 'You select 3, sd -> sd'
			diff /mnt/sd/bigfile /mnt/sd/bigfile-back
			;;
		4)  echo 'You select 4, sd ->nand'
			diff /mnt/sd/bigfile /bigfile-back
			;;
		*) echo 'choice err ,exit!!'
			exit
			;;
	esac
	TMP=$?
	if [ $TMP -eq 0 ]; then
		echo "diff right "
		echo ""
	else
		echo " diff error::::$CNT "
		exit
	fi
}

function DelFile()
{
	rm -rf /bigfile
	rm -rf /bigfile-back
	rm -rf /mnt/sd/bigfile
	rm -rf /mnt/sd/bigfile-back
}

function PlusCount()
{
	CNT=`busybox expr $CNT + 1`
	echo "NAND_COPY_TEST: sleep 1s for recycle::::::::::::::::::::::::::::::::::::::::cnt=$CNT" > /dev/console
	sleep 1
}

# main
DelFile
if [ "$test_module" = "1" ]; then
	while [ 1 ]; do
		GetRandomSize
		GetBigfile $?
		CopyBigfile $copy_module
		sync
		DiffFile $copy_module
		DelFile
		PlusCount
	done
else
	echo "***************************************************"
	echo "please input how many Kbytes you want test at a time."
	echo
	read SIZE

	while [ 1 ];do
		if [ $SIZE -gt $DATA_FREECAP ]; then
			echo "you input $SIZE"
			echo "the free size is $DATA_FREECAP"
			echo "there is not enough disk space, please input again ."
			read SIZE
		fi
		KSIZE=`busybox expr $SIZE \* 1024`
		echo "NAND_COPY_TEST:  size = $KSIZE "
		GetBigfile $KSIZE
		CopyBigfile $copy_module
		sync
		DiffFile $copy_module
		DelFile
		PlusCount
	done
fi
# main end
