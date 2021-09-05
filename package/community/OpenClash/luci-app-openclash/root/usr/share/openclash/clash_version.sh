#!/bin/sh

set_lock() {
   exec 884>"/tmp/lock/openclash_clash_version.lock" 2>/dev/null
   flock -x 884 2>/dev/null
}

del_lock() {
   flock -u 884 2>/dev/null
   rm -rf "/tmp/lock/openclash_clash_version.lock"
}

CKTIME=$(date "+%Y-%m-%d-%H")
LAST_OPVER="/tmp/clash_last_version"
set_lock

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
del_lock