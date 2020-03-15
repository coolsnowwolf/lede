#!/bin/sh
   START_LOG="/tmp/openclash_start.log"
   LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
   LOG_FILE="/tmp/openclash.log"
   echo "开始下载 GEOIP 数据库..." >$START_LOG
   curl -sL --connect-timeout 10 --retry 2 https://static.clash.to/GeoIP2/GeoIP2-Country.mmdb -o /tmp/Country.mmdb >/dev/null 2>&1
   if [ "$?" -eq "0" ] && [ -s "/tmp/Country.mmdb" ]; then
      echo "GEOIP 数据库下载成功，检查数据库版本是否更新..." >$START_LOG
      cmp -s /tmp/Country.mmdb /etc/openclash/Country.mmdb
         if [ "$?" -ne "0" ]; then
            /etc/init.d/openclash stop
            echo "数据库版本有更新，开始替换数据库版本..." >$START_LOG\
            && mv /tmp/Country.mmdb /etc/openclash/Country.mmdb >/dev/null 2>&1\
            && /etc/init.d/openclash start\
            && echo "删除下载缓存..." >$START_LOG\
            && rm -rf /tmp/Country.mmdb >/dev/null 2>&1
            echo "GEOIP 数据库更新成功！" >$START_LOG
            echo "${LOGTIME} GEOIP Database Update Successful" >>$LOG_FILE
            sleep 10
            echo "" >$START_LOG
         else
            echo "数据库版本没有更新，停止继续操作..." >$START_LOG
            echo "${LOGTIME} Updated GEOIP Database No Change, Do Nothing" >>$LOG_FILE
            rm -rf /tmp/Country.mmdb >/dev/null 2>&1
            sleep 5
            echo "" >$START_LOG
         fi
   else
      echo "GEOIP 数据库下载失败，请检查网络或稍后再试！" >$START_LOG
      rm -rf /tmp/Country.mmdb >/dev/null 2>&1
      echo "${LOGTIME} GEOIP Database Update Error" >>$LOG_FILE
      sleep 10
      echo "" >$START_LOG
   fi