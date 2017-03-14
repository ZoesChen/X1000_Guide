#!/bin/sh

buf=$(ifconfig wlan0 | grep "inet addr")

ifconfig wlan0 down
sleep 1

echo /firmware/fw_bcm43438_apsta.bin > /sys/module/bcmdhd/parameters/firmware_path

ifconfig wlan0 up
sleep 1

if [ ! -e /var/lib/misc ]
then
    mkdir -p /var/lib/misc
fi

if [ ! -e /var/lib/misc/udhcpd.leases ]
then
    touch /var/lib/misc/udhcpd.leases
    chmod +x /var/lib/misc/udhcpd.leases
fi

hostapd /etc/hostapd.conf &

ifconfig wlan0 192.168.1.1 netmask 255.255.255.0

udhcpd -fS /etc/udhcpd.conf &

echo "wifi AP finshed!"

