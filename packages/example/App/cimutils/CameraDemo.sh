
echo "1. take picture in usertrp model!!!!"
echo "2. take picture in mmap model!!!!"
echo "3. preview picture to fb in usertrp model!!!"
echo "4. preview picture to fb in mmap model!!!"
echo "please Select a label for operation......For example 1"
read answer

[ -e '/dev/video1' ] && cim_num=1 || cim_num=0

case $answer in
"1")
	cimutils -I $cim_num -C  -x 320 -y 240 -f ingenic_userptr.jpg -l 10 -v
	;;
"2")
	cimutils -I $cim_num -C  -x 320 -y 240 -f ingenic_mmap.jpg -l 10
	;;
"3")
	cimutils -I $cim_num -P -w 320 -h 240  -v
	;;
"4")
	cimutils -I $cim_num -P -w 320 -h 240
	;;
*)
	echo "please check your input"
	;;
esac
