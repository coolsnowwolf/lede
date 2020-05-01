#!/bin/sh
LOCK_FILE="/var/lock/ssr-chinaipset.lock"
[ -f "$LOCK_FILE" ] && exit 2
touch "$LOCK_FILE"
echo "create china hash:net family inet hashsize 1024 maxelem 65536" >/tmp/china.ipset
awk '!/^$/&&!/^#/{printf("add china %s'" "'\n",$0)}' /etc/ssr/china_ssr.txt >>/tmp/china.ipset
ipset -! flush china
ipset -! restore </tmp/china.ipset 2>/dev/null
rm -f /tmp/china.ipset $LOCK_FILE
