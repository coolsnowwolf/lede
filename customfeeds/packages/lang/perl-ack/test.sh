#!/bin/sh
case "$1" in
	"ack")
		ack --version | grep "$PKG_VERSION"
		;;
esac
