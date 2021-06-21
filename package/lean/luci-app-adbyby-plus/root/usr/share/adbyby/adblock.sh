#!/bin/sh

uclient-fetch --no-check-certificate -O - 'https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt' > /tmp/adnew.conf
if [ -s "/tmp/adnew.conf" ];then
  /usr/share/adbyby/ad-update
fi

rm -f /tmp/adbyby.updated 
sleep 10
/etc/init.d/adbyby restart
