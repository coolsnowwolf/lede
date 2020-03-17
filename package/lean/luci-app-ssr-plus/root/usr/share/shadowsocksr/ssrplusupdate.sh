#!/bin/sh

/usr/bin/lua /usr/share/shadowsocksr/update.lua
/usr/bin/lua /usr/share/shadowsocksr/subscribe.lua
sleep 10
/etc/init.d/shadowsocksr restart