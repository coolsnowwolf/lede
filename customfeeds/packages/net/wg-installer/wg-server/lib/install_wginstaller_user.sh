#!/bin/sh

# do not override already existing user!!!
[ "$(uci show rpcd | grep wginstaller)" ] && exit 0

# install wginstaller user with standard credentials
# user: wginstaller
# password: wginstaller
uci add rpcd login
uci set rpcd.@login[-1].username='wginstaller'

password=$(uhttpd -m wginstaller)
uci set rpcd.@login[-1].password=$password
uci add_list rpcd.@login[-1].read='wginstaller'
uci add_list rpcd.@login[-1].write='wginstaller'
uci commit rpcd

# restart rpcd
/etc/init.d/rpcd restart

# restart uhttpd
/etc/init.d/uhttpd restart
