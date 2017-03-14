#!/usr/bin/env python

import os
import sys


if __name__ == '__main__':
	if len(sys.argv) < 4:
		print "arg error: >>> you should exec: %s arch platform board \n" % sys.argv[0]
		raise IndexError

	dir_name = os.path.dirname(sys.argv[0])
	board_str = "export BOARD=" + sys.argv[3]
	hardware_str = "export BOARD_HARDWARE=$hardware"
	platform_str = "export BOARD_PLATFORM=" + sys.argv[2]
	arch_str = "export BOARD_ARCH=" + sys.argv[1]
	FD = open(dir_name + os.sep +"hardware.sh","w")
	FD.writelines(("#!/bin/bash" + os.linesep))
	FD.writelines(("hardware=`echo $(cat /proc/cpuinfo|grep \"Hardware\"|uniq|awk -F : '{print $2}')`" + os.linesep))
	print >> FD,os.linesep
	print >> FD,board_str
	print >> FD,hardware_str
	print >> FD,platform_str
	print >> FD,arch_str
	FD.close()
	os.system(("chmod +x %s" % dir_name + os.sep + "hardware.sh"))
