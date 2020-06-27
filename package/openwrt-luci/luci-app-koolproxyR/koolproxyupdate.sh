wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR_rule_list/master/kp.dat' -O files/usr/share/koolproxy/data/rules/kp.dat
wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR/master/koolproxyR/koolproxyR/data/rules/yhosts.txt' -O files/usr/share/koolproxy/data/rules/yhosts.txt
wget 'https://dev.tencent.com/u/shaoxia1991/p/cjxlist/git/raw/master/cjx-annoyance.txt' -O files/usr/share/koolproxy/data/rules/fanboy.txt
wget 'https://easylist-downloads.adblockplus.org/easylistchina.txt' -O files/usr/share/koolproxy/data/rules/easylistchina.txt

wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR/master/koolproxyR/koolproxyR/data/rules/user.txt' -O files/usr/share/koolproxy/data/user.txt
cp files/usr/share/koolproxy/data/user.txt files/usr/share/koolproxy/data/rules/user.txt
wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR/master/koolproxyR/koolproxyR/data/koolproxyR_ipset.conf' -O files/usr/share/koolproxy/koolproxy_ipset.conf

wget https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt -O- | grep ^\|\|[^\*]*\^$ | sed -e 's:||:address\=\/:' -e 's:\^:/0\.0\.0\.0:' > files/usr/share/koolproxy/dnsmasq.adblock
sed -i '/youku/d' files/usr/share/koolproxy/dnsmasq.adblock
sed -i '/[1-9]\{1,3\}\.[1-9]\{1,3\}\.[1-9]\{1,3\}\.[1-9]\{1,3\}/d' files/usr/share/koolproxy/dnsmasq.adblock

