#!/bin/sh
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
   uci set openclash.config.enable=1
   uci commit openclash
   /usr/share/openclash/openclash_core.sh "$1" >/dev/null 2>&1 &
   /usr/share/openclash/openclash_core.sh "TUN" "$1" >/dev/null 2>&1 &
   /usr/share/openclash/openclash_core.sh "Game" "$1" >/dev/null 2>&1 &
   wait
fi

LAST_OPVER="/tmp/openclash_last_version"
LAST_VER=$(sed -n 1p "$LAST_OPVER" 2>/dev/null |sed "s/^v//g")
OP_CV=$(sed -n 1p /usr/share/openclash/res/openclash_version 2>/dev/null |awk -F '-' '{print $1}' |awk -F 'v' '{print $2}' |awk -F '.' '{print $2$3}' 2>/dev/null)
OP_LV=$(sed -n 1p $LAST_OPVER 2>/dev/null |awk -F '-' '{print $1}' |awk -F 'v' '{print $2}' |awk -F '.' '{print $2$3}' 2>/dev/null)
set_lock

if [ "$(expr "$OP_LV" \> "$OP_CV")" -eq 1 ] && [ -f "$LAST_OPVER" ]; then
   LOG_OUT "Start Downloading【OpenClash - v$LAST_VER】..."
   if pidof clash >/dev/null; then
      curl -sL -m 10 --retry 2 https://github.com/vernesong/OpenClash/releases/download/v"$LAST_VER"/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk >/dev/null 2>&1
   fi
   if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
      curl -sL -m 10 --retry 2 https://cdn.jsdelivr.net/gh/vernesong/OpenClash@master/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "/tmp/openclash.ipk" ]; then
      LOG_OUT "【OpenClash - v$LAST_VER】Download Successful, Start Pre Update Test..."
      opkg install /tmp/openclash.ipk --noaction >>$LOG_FILE
      if [ "$?" -ne "0" ]; then
         LOG_OUT "【OpenClash - v$LAST_VER】Pre Update Test Failed, The File is Saved in /tmp/opencrash.ipk, Please Try to Update Manually!"
         sleep 10
         SLOG_CLEAN
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
uci set openclash.config.enable=0
uci commit openclash
opkg remove --force-depends --force-remove luci-app-openclash
LOG_OUT "Installing The New Version, Please Do Not Refresh The Page or Do Other Operations..."
opkg install /tmp/openclash.ipk
if [ "$?" -eq "0" ]; then
   rm -rf /tmp/openclash.ipk >/dev/null 2>&1
   LOG_OUT "OpenClash Update Successful, About To Restart!"
   sleep 3
   uci set openclash.config.enable=1
   uci commit openclash
   /etc/init.d/openclash restart 2>/dev/null
else
   LOG_OUT "OpenClash Update Failed, The File is Saved in /tmp/openclash.ipk, Please Try to Update Manually!"
   sleep 10
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
      sleep 5
      SLOG_CLEAN
      if [ "$(uci get openclash.config.config_reload 2>/dev/null)" -eq 0 ]; then
         uci set openclash.config.config_reload=1
         uci commit openclash
      	 /etc/init.d/openclash restart 2>/dev/null
      fi
   fi
else
   if [ ! -f "$LAST_OPVER" ]; then
      LOG_OUT "Failed to Get Version Information, Please Try Again Later..."
      sleep 5
      SLOG_CLEAN
   else
      LOG_OUT "OpenClash Has not Been Updated, Stop Continuing!"
      sleep 5
      SLOG_CLEAN
   fi
   if [ "$(uci get openclash.config.config_reload 2>/dev/null)" -eq 0 ]; then
      uci set openclash.config.config_reload=1
      uci commit openclash
      /etc/init.d/openclash restart 2>/dev/null
   fi
fi
del_lock