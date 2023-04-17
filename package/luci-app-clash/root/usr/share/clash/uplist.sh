#!/bin/bash /etc/rc.common
. /lib/functions.sh
 
name=$(uci get clash.config.config_up_remove 2>/dev/null)
if [ ! -z $name ];then
rm -rf /usr/share/clash/config/upload/${name}
fi
