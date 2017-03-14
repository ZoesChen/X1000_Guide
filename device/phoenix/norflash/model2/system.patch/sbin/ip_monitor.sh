udhcpc -i $1 -q
sleep 5
/sbin/pinggetway.sh
if [ $?==0 ]; then
/sbin/net_monitor wlan0 &
fi
/sbin/send_random
while [ 1 ]; do
	sleep 30
	/sbin/pinggetway.sh
	if [ $?==1 ]; then
		exit 1
	fi
done
