#!/bin/sh

killall hostapd
killall udhcpd
killall wpa_supplicant


ifconfig wlan0 down

echo /firmware/fw > /sys/module/bcmdhd/parameters/firmware_path

#ifconfig wlan0 up
#sleep 1

wpa_supplicant -iwlan0 -Dnl80211 -c/etc/wpa_supplicant.conf -B

sleep 1

udhcpc -iwlan0
