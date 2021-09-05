#!/bin/sh

case "$1" in
	php7-cgi)
		php-cgi -v | grep "$2"
		;;
	php7-cli)
		php-cli -v | grep "$2"
		;;
	php7-fpm)
		php-fpm -v | grep "$2"
		;;
	php7-mod-*)
		PHP_MOD="${1#php7-mod-}"
		PHP_MOD="${PHP_MOD//-/_}"

		opkg install php7-cli

		php-cli -m | grep -i "$PHP_MOD"
		;;
	*)
		;;
esac
