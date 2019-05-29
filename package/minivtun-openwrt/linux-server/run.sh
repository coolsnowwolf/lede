#!/bin/bash

# This script will be set up minivtun

# for ipv4
export server=0.0.0.0
# for ipv6
# export server=[::] 
export server_port=456
export local_addr=10.7.0.1/24
export password=password
export mtu=1300
export intf=mv0
export pid_file=/var/run/minivtun.pid

if [ $EUID -ne 0 ];then
	echo "Please run as root"
	exit 1
fi

echo "1 for run, 2 for stop"
read choose

if [ x$choose = "x1" ];then
	if [ -f $pid_file ];then
		echo minivtun is already running
		exit 1
	fi
	
	minivtun -l $server:$server_port -a $local_addr -m $mtu -n $intf -p $pid_file -e $password -d && \
	sh server_up.sh
else
	if [ ! -f $pid_file ];then
		echo minivtun is not running yet
		exit 1
	fi
	
	sh server_down.sh && \
	kill -9 `cat $pid_file` && \
	rm $pid_file
fi
