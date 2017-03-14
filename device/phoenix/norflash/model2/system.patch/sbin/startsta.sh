#!/bin/sh

DHCP=yes
INTERFACE=wlan0
if [ $# -ge 1 -a "$1" == "--without-dhcp" ]; then
	DHCP=no
fi

# stop already exist process
killall udhcpc > /dev/null
killall udhcpd > /dev/null
killall wpa_supplicant > /dev/null
killall hostapd  > /dev/null

#killall render > /dev/null
#killall player > /dev/null
#killall localplayer > /dev/null
#killall shairport > /dev/null

ifconfig $INTERFACE up

sleep 2
# excute airkiss core function
/sbin/akiss -k 0123456789123456

ifconfig $INTERFACE down

# wpa_supplicant config file
if [ -f /etc/wpa_conf.conf ]; then
	WPA_CONF=/etc/wpa_conf.conf
else
	WPA_CONF=/etc/wpa_supplicant.conf
fi

# guess what wifi model we are using(light detect, may not match!!!)

#if [ -d /proc/net/rtl*/wlan? ]; then
#	INTERFACE=`basename /proc/net/rtl*/wlan?`
#fi

# delete default Gateway
route del default gw 0.0.0.0 dev $INTERFACE
# release ip address
ifconfig $INTERFACE 0.0.0.0
# turn up wifi interface
ifconfig $INTERFACE up

# start service
wpa_supplicant -Dnl80211 -i$INTERFACE -c$WPA_CONF -B

if [ "$DHCP" == "yes" ]; then
	ip_monitor.sh $INTERFACE  &
fi

# Add Multicast Router for Apple Airplay
#route add -net 224.0.0.0 netmask 224.0.0.0 $INTERFACE

#render &
#player &
#localplayer &
#shairport -a "player on Ingenic" -d

exit 0
