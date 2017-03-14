var=$(ip route show)
i=1
while((i==1))
do
	split=`echo $var|cut -d " " -f$i`
	let "i+=1"
	if [ $i == 4 ]
	then
		break
	fi
done
ping -c 10 $split  > /dev/null

if [ $? != 0 ]; then
	echo "bad net environment wait 10 seconds try again"
	sleep 10
	ping -c 10 $split  > /dev/null
fi

if [ $? != 0 ]; then
	echo "time out to kill"
	killall wpa_supplicant /dev/null
	killall udhcpc /dev/null
	ifconfig wlan0 down
	killall net_monitor
	echo 0 > /sys/class/leds/wl_led_r/brightness
	exit 1
fi
