#!/bin/bash  
  
for((i=1;i<=200;i++));  
do   
  uci delete shadowsocksr.@servers[$1] >/dev/null 2>&1
done
uci commit shadowsocksr

/etc/init.d/shadowsocksr stop