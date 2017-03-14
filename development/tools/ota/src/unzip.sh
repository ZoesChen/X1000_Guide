#!/bin/sh

timeout=1800;
line=0;

fdo_error()
{
    rm -rf /tmp/update*;
    echo "$1; wait 2s;reconnect";
    sleep 2;
    let timeout=$timeout-1;
}

unzip_data()
{
    mkdir -p /tmp/updatezip
    cd /tmp/updatezip
    unzip /tmp/updatezip$update_num.zip;
    if [ $? -eq 0 ]
    then
	cd /tmp/updatezip/updatezip$update_num;
	. update.script;
	cd /;
	rm -rf /tmp/updatezip*;
	return 0;
    else
	rm -rf /tmp/updatezip*;
	fdo_error "unzip error";
    fi

    return 1;
}

fmain()
{
    update_num=$1;

    echo "update num is $update_num";

    while [ $timeout -ne 0 ]
    do
	unzip_data $timeout;
	if [ $? -eq 0 ]
	then
	    break;
	fi
    done

    if [ $timeout -eq 0 ]
    then
	echo "poweroff";
	poweroff;
	while [ true ]
	do
	    sleep 2
	done
    fi
}
fmain $1
