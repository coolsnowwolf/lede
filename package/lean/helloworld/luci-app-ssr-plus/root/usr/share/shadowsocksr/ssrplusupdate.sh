#!/bin/sh
/usr/bin/lua /usr/share/shadowsocksr/update.lua
sleep 2s
/usr/share/shadowsocksr/chinaipset.sh /var/etc/ssrplus/china_ssr.txt
sleep 2s
/usr/bin/lua /usr/share/shadowsocksr/subscribe.lua
