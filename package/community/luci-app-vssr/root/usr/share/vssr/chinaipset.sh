echo "create china hash:net family inet hashsize 1024 maxelem 65536" >/tmp/china.ipset
awk '!/^$/&&!/^#/{printf("add china %s'" "'\n",$0)}' /etc/vssr/china_ssr.txt >>/tmp/china.ipset
ipset -! flush china
ipset -! restore </tmp/china.ipset 2>/dev/null
rm -f /tmp/china.ipset
