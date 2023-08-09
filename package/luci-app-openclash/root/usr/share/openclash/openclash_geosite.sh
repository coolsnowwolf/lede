#!/bin/bash
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/log.sh

   set_lock() {
      exec 874>"/tmp/lock/openclash_geosite.lock" 2>/dev/null
      flock -x 874 2>/dev/null
   }

   del_lock() {
      flock -u 874 2>/dev/null
      rm -rf "/tmp/lock/openclash_geosite.lock"
   }

   small_flash_memory=$(uci get openclash.config.small_flash_memory 2>/dev/null)
   GEOSITE_CUSTOM_URL=$(uci get openclash.config.geosite_custom_url 2>/dev/null)
   github_address_mod=$(uci -q get openclash.config.github_address_mod || echo 0)
   LOG_FILE="/tmp/openclash.log"
   restart=0
   set_lock
   
   if [ "$small_flash_memory" != "1" ]; then
   	  geosite_path="/etc/openclash/GeoSite.dat"
   	  mkdir -p /etc/openclash
   else
   	  geosite_path="/tmp/etc/openclash/GeoSite.dat"
   	  mkdir -p /tmp/etc/openclash
   fi
   LOG_OUT "Start Downloading GeoSite Database..."
   if [ -z "$GEOSITE_CUSTOM_URL" ]; then
      if [ "$github_address_mod" != "0" ]; then
         if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"gh/Loyalsoldier/v2ray-rules-dat@release/geosite.dat -o /tmp/GeoSite.dat 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/GeoSite.dat" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.fastgit.org/Loyalsoldier/v2ray-rules-dat/release/geosite.dat -o /tmp/GeoSite.dat 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/GeoSite.dat" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         else
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geosite.dat -o /tmp/GeoSite.dat 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/GeoSite.dat" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         fi
      else
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geosite.dat -o /tmp/GeoSite.dat 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/GeoSite.dat" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
      if [ "${PIPESTATUS[0]}" -ne "0" ]; then
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "https://ftp.jaist.ac.jp/pub/sourceforge.jp/storage/g/v/v2/v2raya/dists/v2ray-rules-dat/geosite.dat" -o /tmp/GeoSite.dat 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/GeoSite.dat" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
   else
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$GEOSITE_CUSTOM_URL" -o /tmp/GeoSite.dat 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/GeoSite.dat" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   fi
   if [ "${PIPESTATUS[0]}" -eq "0" ] && [ -s "/tmp/GeoSite.dat" ]; then
      LOG_OUT "GeoSite Database Download Success, Check Updated..."
      cmp -s /tmp/GeoSite.dat "$geosite_path"
      if [ "$?" -ne "0" ]; then
         LOG_OUT "GeoSite Database Has Been Updated, Starting To Replace The Old Version..."
         rm -rf "/etc/openclash/geosite.dat"
         mv /tmp/GeoSite.dat "$geosite_path" >/dev/null 2>&1
         LOG_OUT "GeoSite Database Update Successful!"
         restart=1
      else
         LOG_OUT "Updated GeoSite Database No Change, Do Nothing..."
      fi
   else
      LOG_OUT "GeoSite Database Update Error, Please Try Again Later..."
   fi

   if [ "$restart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && [ "$(find /tmp/lock/ |grep -v "openclash.lock" |grep -c "openclash")" -le 1 ]; then
      /etc/init.d/openclash restart >/dev/null 2>&1 &
   elif [ "$restart" -eq 0 ] && [ "$(unify_ps_prevent)" -eq 0 ] && [ "$(find /tmp/lock/ |grep -v "openclash.lock" |grep -c "openclash")" -le 1 ] && [ "$(uci -q get openclash.config.restart)" -eq 1 ]; then
      /etc/init.d/openclash restart >/dev/null 2>&1 &
      uci -q set openclash.config.restart=0
      uci -q commit openclash
   elif [ "$restart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ]; then
      uci -q set openclash.config.restart=1
      uci -q commit openclash
   fi

   rm -rf /tmp/GeoSite.dat >/dev/null 2>&1
   SLOG_CLEAN
   del_lock
