#!/bin/sh

echo "--test wifi read configuration file --"

kill -9 $(pidof udhcpc wpa_supplicant)
cd
cd /etc
if test -e wpa_supplicant
then rm wpa_supplicant
	if test -e wpa_conf.conf
	then rm wpa_conf.conf
	else echo
	fi
	touch wpa_supplicant
else echo
fi

[ ! -e "/etc/wpa_conf.conf" ] && {
cat > /etc/wpa_conf.conf << EOF
ctrl_interface=/var/run/wpa_supplicant
ap_scan=1
network={
ssid="JZ_MD"
proto=WPA2
psk="1JZmdingenic2"
}
EOF
}


ifconfig wlan0 up
sleep 3
wpa_supplicant -iwlan0 -Dnl80211 -c/etc/wpa_conf.conf -B
sleep 3
udhcpc -iwlan0
sleep 30
ifconfig |grep wlan0
ping 192.168.1.57 -c 6
ping www.baidu.com -c 6

if [ $? -eq 0 ];
then
	echo "--Wifi test passed --"
else
	echo "--Wifi test failed --"
 fi
sleep 3

