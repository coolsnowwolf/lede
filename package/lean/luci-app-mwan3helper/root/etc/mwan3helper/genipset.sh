ipset -N $1 hash:net 2>/dev/null

echo "create $1 hash:net family inet hashsize 1024 maxelem 65536" > /tmp/mwan3.ipset
cat $2 | sed -e "s/^/add $1 /" >> /tmp/mwan3.ipset
ipset -! flush $1
ipset -! restore < /tmp/mwan3.ipset 2>/dev/null
rm -f /tmp/mwan3.ipset
