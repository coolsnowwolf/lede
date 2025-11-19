#!/bin/sh

MHz=`grep 'MHz' /proc/cpuinfo | cut -c11- |sed -n '1p'`

if [ "$(grep 'AMD' /proc/cpuinfo)" != "" ]; then
TEMP=`sensors 2>/dev/null | grep 'Tctl' | cut -c16-`
else
TEMP=`sensors 2>/dev/null | grep '0:' | awk -F ':' '{print $2}'`
fi

echo "$MHz MHz $TEMP "
