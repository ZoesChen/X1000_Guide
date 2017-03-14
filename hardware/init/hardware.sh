#!/bin/bash
hardware=`echo $(cat /proc/cpuinfo|grep "Hardware"|uniq|awk -F : '{print $2}')`


export BOARD=halley2
export BOARD_HARDWARE=$hardware
export BOARD_PLATFORM=x1000
export BOARD_ARCH=mips
