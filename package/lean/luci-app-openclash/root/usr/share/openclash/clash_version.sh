#!/bin/sh
. /usr/share/openclash/openclash_ps.sh

#禁止多个实例
status=$(unify_ps_status "clash_version.sh")
[ "$status" -gt "3" ] && exit 0

CKTIME=$(date "+%Y-%m-%d-%H")
LAST_OPVER="/tmp/clash_last_version"

if [ "$CKTIME" != "$(grep "CheckTime" $LAST_OPVER 2>/dev/null |awk -F ':' '{print $2}')" ]; then
	 if pidof clash >/dev/null; then
      curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/vernesong/OpenClash/master/core_version -o $LAST_OPVER >/dev/null 2>&1
   fi
   if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
      curl -sL --connect-timeout 10 --retry 2 https://cdn.jsdelivr.net/gh/vernesong/OpenClash@master/core_version -o $LAST_OPVER >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "$LAST_OPVER" ]; then
      echo "CheckTime:$CKTIME" >>$LAST_OPVER
   else
      rm -rf $LAST_OPVER
   fi
fi
