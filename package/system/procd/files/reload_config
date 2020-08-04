#!/bin/sh
rm -rf /var/run/config.check
mkdir -p /var/run/config.check
for config in /etc/config/*; do
	file=${config##*/}
	uci show "${file##*/}" > /var/run/config.check/$file
done
MD5FILE=/var/run/config.md5
[ -f $MD5FILE ] && {
	for c in $(md5sum -c $MD5FILE 2>/dev/null| grep FAILED | cut -d: -f1); do
		ubus call service event "{ \"type\": \"config.change\", \"data\": { \"package\": \"$(basename $c)\" }}"
	done
}
md5sum /var/run/config.check/* > $MD5FILE
rm -rf /var/run/config.check
