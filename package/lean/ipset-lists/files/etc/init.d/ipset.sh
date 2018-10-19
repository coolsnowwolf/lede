#!/bin/sh /etc/rc.common
# Copyright (C) 2014 Justin Liu <rssnsj@gmail.com>

START=21

start()
{
	local file
	for file in /etc/ipset/*; do
		[ -f $file ] || continue
		ipset restore < $file
	done
}

stop()
{
	local file
	for file in /etc/ipset/*; do
		[ -f $file ] || continue
		ipset destroy `basename $file`
	done
}

restart()
{
	stop >/dev/null 2>&1
	start
}

