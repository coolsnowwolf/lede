#!/bin/bash

set_lock() {
   exec 884>"/tmp/lock/openclash_clash_version.lock" 2>/dev/null
   flock -x 884 2>/dev/null
}

del_lock() {
   flock -u 884 2>/dev/null
   rm -rf "/tmp/lock/openclash_clash_version.lock"
}

TIME=$(date "+%Y-%m-%d-%H")
CHTIME=$(date "+%Y-%m-%d-%H" -r "/tmp/clash_last_version" 2>/dev/null)
LAST_OPVER="/tmp/clash_last_version"
RELEASE_BRANCH=$(uci -q get openclash.config.release_branch || echo "master")
github_address_mod=$(uci -q get openclash.config.github_address_mod || echo 0)
LOG_FILE="/tmp/openclash.log"
set_lock

if [ "$TIME" != "$CHTIME" ]; then
   if [ "$github_address_mod" != "0" ]; then
      if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
         curl -SsL -m 60 "$github_address_mod"gh/vernesong/OpenClash@core/"$RELEASE_BRANCH"/core_version -o $LAST_OPVER 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$LAST_OPVER" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
         curl -SsL -m 60 https://raw.fastgit.org/vernesong/OpenClash/core/"$RELEASE_BRANCH"/core_version -o $LAST_OPVER 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$LAST_OPVER" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      else
         curl -SsL -m 60 "$github_address_mod"https://raw.githubusercontent.com/vernesong/OpenClash/core/"$RELEASE_BRANCH"/core_version -o $LAST_OPVER 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$LAST_OPVER" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
   else
      curl -SsL -m 60 https://raw.githubusercontent.com/vernesong/OpenClash/core/"$RELEASE_BRANCH"/core_version -o $LAST_OPVER 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$LAST_OPVER" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   fi
   
   if [ "${PIPESTATUS[0]}" -ne 0 ] || [ -n "$(cat $LAST_OPVER |grep '<html>')" ]; then
      curl -SsL -m 60 --retry 2 https://ftp.jaist.ac.jp/pub/sourceforge.jp/storage/g/o/op/openclash/"$RELEASE_BRANCH"/core_version -o $LAST_OPVER 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$LAST_OPVER" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      curl_status=${PIPESTATUS[0]}
   else
      curl_status=0
   fi
   
   if [ "$curl_status" -ne 0 ] ; then
      rm -rf $LAST_OPVER
   fi
fi
del_lock
