#!/bin/sh
#禁止多个实例
status=$(ps|grep -c /usr/share/openclash/openclash_update.sh)
[ "$status" -gt "3" ] && exit 0

START_LOG="/tmp/openclash_start.log"
LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
LOG_FILE="/tmp/openclash.log"
LAST_OPVER="/tmp/openclash_last_version"
LAST_VER=$(sed -n 1p "$LAST_OPVER" 2>/dev/null |sed "s/^v//g")
OP_CV=$(sed -n 1p /etc/openclash/openclash_version 2>/dev/null |awk -F '-' '{print $1}' |awk -F 'v' '{print $2}' |awk -F '.' '{print $2$3}' 2>/dev/null)
OP_LV=$(sed -n 1p $LAST_OPVER 2>/dev/null |awk -F '-' '{print $1}' |awk -F 'v' '{print $2}' |awk -F '.' '{print $2$3}' 2>/dev/null)
HTTP_PORT=$(uci get openclash.config.http_port 2>/dev/null)
PROXY_ADDR=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)
if [ -s "/tmp/openclash.auth" ]; then
   PROXY_AUTH=$(cat /tmp/openclash.auth |awk -F '- ' '{print $2}' |sed -n '1p' 2>/dev/null)
fi

if [ "$(expr "$OP_LV" \> "$OP_CV")" -eq 1 ] && [ -f "$LAST_OPVER" ]; then
   echo "开始下载 OpenClash-$LAST_VER ..." >$START_LOG
   if pidof clash >/dev/null; then
      curl -sL -m 30 --retry 5 -x http://$PROXY_ADDR:$HTTP_PORT -U "$PROXY_AUTH" https://github.com/vernesong/OpenClash/releases/download/v"$LAST_VER"/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk >/dev/null 2>&1
   else
      curl -sL -m 30 --retry 5 https://github.com/vernesong/OpenClash/releases/download/v"$LAST_VER"/luci-app-openclash_"$LAST_VER"_all.ipk -o /tmp/openclash.ipk >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "/tmp/openclash.ipk" ]; then
      echo "OpenClash-$LAST_VER 下载成功，开始更新，更新过程请不要刷新页面和进行其他操作..." >$START_LOG
      cat > /tmp/openclash_update.sh <<"EOF"
#!/bin/sh
LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
START_LOG="/tmp/openclash_start.log"
LOG_FILE="/tmp/openclash.log"
echo "正在卸载旧版本，更新过程请不要刷新页面和进行其他操作 ..." >$START_LOG
uci set openclash.config.enable=0
uci commit openclash
opkg remove luci-app-openclash
echo "正在安装新版本，更新过程请不要刷新页面和进行其他操作 ..." >$START_LOG
opkg install /tmp/openclash.ipk
if [ "$?" -eq "0" ]; then
   rm -rf /tmp/openclash.ipk >/dev/null 2>&1
   echo "OpenClash 更新成功，即将进行重启！" >$START_LOG
   echo "${LOGTIME} OpenClash Update Successful" >>$LOG_FILE
   sleep 3
   /etc/init.d/openclash restart 2>/dev/null
else
   echo "OpenClash 更新失败，文件保存在/tmp/openclash.ipk，请尝试手动更新！" >$START_LOG
   echo "${LOGTIME} OpenClash Update Fail" >>$LOG_FILE
   sleep 10
   echo "" >$START_LOG
fi
EOF
   chmod 4755 /tmp/openclash_update.sh
   nohup /tmp/openclash_update.sh &
   wait
   rm -rf /tmp/openclash_update.sh
   else
      echo "OpenClash-$LAST_VER 下载失败，请检查网络或稍后再试！" >$START_LOG
      rm -rf /tmp/openclash.ipk >/dev/null 2>&1
      echo "${LOGTIME} OpenClash Update Error" >>$LOG_FILE
      sleep 10
      echo "" >$START_LOG
   fi
elif [ ! -f "$LAST_OPVER" ]; then
      echo "获取版本信息失败，请稍后再试..." >$START_LOG
      echo "${LOGTIME} OpenClash Version Check Error, Please Try Again After A few seconds" >>$LOG_FILE
      sleep 5
      echo "" >$START_LOG
else
      echo "OpenClash 没有更新，停止继续操作！" >$START_LOG
      echo "${LOGTIME} OpenClash Version No Change, Do Nothing" >>$LOG_FILE
      sleep 5
      echo "" >$START_LOG
fi
