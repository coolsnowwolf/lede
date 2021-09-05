#!/bin/sh

case "$1" in
	php8-cgi)
		php8-cgi -v | grep "$2"
		;;
	php8-cli)
		php8-cli -v | grep "$2"
		;;
	php8-fpm)
		php8-fpm -v | grep "$2"
		;;
	php8-mod-*)
		PHP_MOD="${1#php8-mod-}"
		PHP_MOD="${PHP_MOD//-/_}"

		opkg install php8-cli

		php8-cli -m | grep -i "$PHP_MOD"
		;;
	*)
		;;
esac
