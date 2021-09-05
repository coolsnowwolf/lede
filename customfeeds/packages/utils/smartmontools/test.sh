#!/bin/sh

case "$1" in
	smartctl|smartd) "$1" -V | head -n 1 | grep "$2" ;;
esac
