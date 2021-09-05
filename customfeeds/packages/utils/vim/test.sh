#!/bin/sh

case "$1" in
	vim|vim-full|vim-fuller)
		vim --version | grep "$2"
		;;
esac
