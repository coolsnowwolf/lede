#!/bin/sh
. /usr/share/openclash/openclash_ps.sh

   status=$(unify_ps_status "openclash_ipdb.sh")
   [ "$status" -gt 3 ] && exit 0

   START_LOG="/tmp/openclash_start.log"
   LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
   LOG_FILE="/tmp/openclash.log"
   small_flash_memory=$(uci get openclash.config.small_flash_memory 2>/dev/null)
   
   if [ "$small_flash_memory" != "1" ]; then
   	  geoip_path="/etc/openclash/Country.mmdb"
   	  mkdir -p /etc/openclash
   else
   	  geoip_path="/tmp/etc/openclash/Country.mmdb"
   	  mkdir -p /tmp/etc/openclash
   fi
   echo "开始下载 GEOIP 数据库..." >$START_LOG
   if pidof clash >/dev/null; then
      curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/alecthw/mmdb_china_ip_list/release/Country.mmdb -o /tmp/Country.mmdb >/dev/null 2>&1
   fi
   if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
      curl -sL --connect-timeout 10 --retry 2 http://www.ideame.top/mmdb/Country.mmdb -o /tmp/Country.mmdb >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "/tmp/Country.mmdb" ]; then
      echo "GEOIP 数据库下载成功，检查数据库版本是否更新..." >$START_LOG
      cmp -s /tmp/Country.mmdb "$geoip_path"
         if [ "$?" -ne "0" ]; then
            echo "数据库版本有更新，开始替换数据库版本..." >$START_LOG
            mv /tmp/Country.mmdb "$geoip_path" >/dev/null 2>&1
            echo "删除下载缓存..." >$START_LOG
            rm -rf /tmp/Country.mmdb >/dev/null 2>&1
            echo "GEOIP 数据库更新成功！" >$START_LOG
            echo "${LOGTIME} GEOIP Database Update Successful" >>$LOG_FILE
            sleep 5
            [ "$(unify_ps_prevent)" -eq 0 ] && /etc/init.d/openclash restart >/dev/null 2>&1 &
         else
            echo "数据库版本没有更新，停止继续操作..." >$START_LOG
            echo "${LOGTIME} Updated GEOIP Database No Change, Do Nothing" >>$LOG_FILE
            rm -rf /tmp/Country.mmdb >/dev/null 2>&1
            sleep 5
         fi
   else
      echo "GEOIP 数据库下载失败，请检查网络或稍后再试！" >$START_LOG
      rm -rf /tmp/Country.mmdb >/dev/null 2>&1
      echo "${LOGTIME} GEOIP Database Update Error" >>$LOG_FILE
      sleep 10
   fi
   echo "" >$START_LOG