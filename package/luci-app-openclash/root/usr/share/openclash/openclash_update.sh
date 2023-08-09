#!/bin/bash
. /usr/share/openclash/log.sh

set_lock() {
   exec 878>"/tmp/lock/openclash_update.lock" 2>/dev/null
   flock -x 878 2>/dev/null
}

del_lock() {
   flock -u 878 2>/dev/null
   rm -rf "/tmp/lock/openclash_update.lock"
}

#一键更新
if [ "$1" = "one_key_update" ]; then
   uci -q set openclash.config.enable=1
   uci -q commit openclash
   /usr/share/openclash/openclash_core.sh "$1" >/dev/null 2>&1 &
   /usr/share/openclash/openclash_core.sh "TUN" "$1" >/dev/null 2>&1 &
   /usr/share/openclash/openclash_core.sh "Meta" "$1" >/dev/null 2>&1 &
   wait
fi

LAST_OPVER="/tmp/openclash_last_version"
LAST_VER=$(sed -n 1p "$LAST_OPVER" 2>/dev/null |sed "s/^v//g" |tr -d "\n")
OP_CV=$(rm -f /var/lock/opkg.lock && opkg status luci-app-openclash 2>/dev/null |grep 'Version' |awk -F '-' '{print $1}' |awk -F 'Version: ' '{print $2}' |awk -F '.' '{print $2$3}' 2>/dev/null)
OP_LV=$(sed -n 1p "$LAST_OPVER" 2>/dev/null |awk -F '-' '{print $1}' |awk -F 'v' '{print $2}' |awk -F '.' '{print $2$3}' 2>/dev/null)
RELEASE_BRANCH=$(uci -q get openclash.config.release_branch || echo "master")
github_address_mod=$(uci -q get openclash.config.github_address_mod || echo 0)
LOG_FILE="/tmp/openclash.log"
set_lock

if [ -n "$OP_CV" ] && [ -n "$OP_LV" ] && [ "$(expr "$OP_LV" \> "$OP_CV")" -eq 1 ] && [ -f "$LAST_OPVER" ]; then
   LOG_OUT "Start Downloading【OpenClash - v$LAST_VER】..."
   if [ "$github_address_mod" != "0" ]; then
      if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"gh/vernesong/OpenClash@package/"$RELEASE_BRANCH"/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/openclash.ipk" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.fastgit.org/vernesong/OpenClash/package/"$RELEASE_BRANCH"/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/openclash.ipk" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      else
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"https://raw.githubusercontent.com/vernesong/OpenClash/package/"$RELEASE_BRANCH"/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/openclash.ipk" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
   else
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.githubusercontent.com/vernesong/OpenClash/package/"$RELEASE_BRANCH"/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/openclash.ipk" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   fi
   if [ "${PIPESTATUS[0]}" -ne 0 ]; then
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://ftp.jaist.ac.jp/pub/sourceforge.jp/storage/g/o/op/openclash/"$RELEASE_BRANCH"/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/openclash.ipk" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      curl_status=${PIPESTATUS[0]}
   else
      curl_status=0
   fi
   if [ "$curl_status" -eq 0 ] && [ -s "/tmp/openclash.ipk" ]; then
      LOG_OUT "【OpenClash - v$LAST_VER】Download Successful, Start Pre Update Test..."
      
      if [ -z "$(opkg install /tmp/openclash.ipk --noaction 2>/dev/null |grep 'Upgrading luci-app-openclash on root' 2>/dev/null)" ]; then
         LOG_OUT "【OpenClash - v$LAST_VER】Pre Update Test Failed, The File is Saved in /tmp/openclash.ipk, Please Try to Update Manually!"
         if [ "$(uci -q get openclash.config.config_reload)" -eq 0 ]; then
      	    /etc/init.d/openclash restart >/dev/null 2>&1 &
         else
            SLOG_CLEAN
         fi
         del_lock
         exit 0
      fi
      LOG_OUT "【OpenClash - v$LAST_VER】Pre Update Test Passed, Ready to Update and Please Do not Refresh The Page and Other Operations..."
      cat > /tmp/openclash_update.sh <<"EOF"
#!/bin/sh
START_LOG="/tmp/openclash_start.log"
LOG_FILE="/tmp/openclash.log"
LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
		
LOG_OUT()
{
	if [ -n "${1}" ]; then
		echo -e "${1}" > $START_LOG
		echo -e "${LOGTIME} ${1}" >> $LOG_FILE
	fi
}

SLOG_CLEAN()
{
	echo "" > $START_LOG
}

LOG_OUT "Uninstalling The Old Version, Please Do not Refresh The Page or Do Other Operations..."
uci -q set openclash.config.enable=0
uci -q commit openclash
opkg remove --force-depends --force-remove luci-app-openclash
LOG_OUT "Installing The New Version, Please Do Not Refresh The Page or Do Other Operations..."
opkg install /tmp/openclash.ipk
if [ "$?" == "0" ]; then
   rm -rf /tmp/openclash.ipk >/dev/null 2>&1
   LOG_OUT "OpenClash Update Successful, About To Restart!"
   uci -q set openclash.config.enable=1
   uci -q commit openclash
   /etc/init.d/openclash restart 2>/dev/null
else
   LOG_OUT "OpenClash Update Failed, The File is Saved in /tmp/openclash.ipk, Please Try to Update Manually!"
   SLOG_CLEAN
fi
EOF
   chmod 4755 /tmp/openclash_update.sh
   nohup /tmp/openclash_update.sh &
   wait
   rm -rf /tmp/openclash_update.sh
   else
      LOG_OUT "【OpenClash - v$LAST_VER】Download Failed, Please Check The Network or Try Again Later!"
      rm -rf /tmp/openclash.ipk >/dev/null 2>&1
      if [ "$(uci -q get openclash.config.config_reload)" -eq 0 ]; then
      	 /etc/init.d/openclash restart >/dev/null 2>&1 &
      else
         SLOG_CLEAN
      fi
   fi
else
   if [ ! -f "$LAST_OPVER" ] || [ -z "$OP_CV" ] || [ -z "$OP_LV" ]; then
      LOG_OUT "Failed to Get Version Information, Please Try Again Later..."
   else
      LOG_OUT "OpenClash Has not Been Updated, Stop Continuing!"
   fi
   if [ "$(uci -q get openclash.config.config_reload)" -eq 0 ]; then
      /etc/init.d/openclash restart >/dev/null 2>&1 &
   else
      SLOG_CLEAN
   fi
fi
del_lock
