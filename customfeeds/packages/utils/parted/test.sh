#!/bin/sh

case "$1" in
	"partprobe")
		test $(/sbin/partprobe --version | grep '^Copyright' | wc -l) -gt 0
		;;
	"parted")
		test $(/sbin/parted --version | grep '^Copyright' | wc -l) -gt 0
		;;
esac
