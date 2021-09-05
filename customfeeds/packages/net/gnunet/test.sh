#!/bin/sh

case "$1" in
	"gnunet")
		gnunet-arm --version | grep "$2"
		;;
esac
