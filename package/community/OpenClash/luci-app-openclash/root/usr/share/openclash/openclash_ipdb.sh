#!/bin/sh
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/log.sh

   set_lock() {
      exec 880>"/tmp/lock/openclash_ipdb.lock" 2>/dev/null
      flock -x 880 2>/dev/null
   }

   del_lock() {
      flock -u 880 2>/dev/null
      rm -rf "/tmp/lock/openclash_ipdb.lock"
   }

   small_flash_memory=$(uci get openclash.config.small_flash_memory 2>/dev/null)
   GEOIP_CUSTOM_URL=$(uci get openclash.config.geo_custom_url 2>/dev/null)
   set_lock
   
   if [ "$small_flash_memory" != "1" ]; then
   	  geoip_path="/etc/openclash/Country.mmdb"
   	  mkdir -p /etc/openclash
   else
   	  geoip_path="/tmp/etc/openclash/Country.mmdb"
   	  mkdir -p /tmp/etc/openclash
   fi
   LOG_OUT "Start Downloading Geoip Database..."
   if [ -z "$GEOIP_CUSTOM_URL" ]; then
      if pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/alecthw/mmdb_china_ip_list/release/lite/Country.mmdb -o /tmp/Country.mmdb >/dev/null 2>&1
      fi
      if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://cdn.jsdelivr.net/gh/alecthw/mmdb_china_ip_list@release/lite/Country.mmdb -o /tmp/Country.mmdb >/dev/null 2>&1
      fi
   else
      curl -sL --connect-timeout 10 --retry 2 "$GEOIP_CUSTOM_URL" -o /tmp/Country.mmdb >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "/tmp/Country.mmdb" ]; then
      LOG_OUT "Geoip Database Download Success, Check Updated..."
      cmp -s /tmp/Country.mmdb "$geoip_path"
      if [ "$?" -ne "0" ]; then
         LOG_OUT "Geoip Database Has Been Updated, Starting To Replace The Old Version..."
         mv /tmp/Country.mmdb "$geoip_path" >/dev/null 2>&1
         LOG_OUT "Geoip Database Update Successful!"
         sleep 5
         [ "$(unify_ps_prevent)" -eq 0 ] && /etc/init.d/openclash restart >/dev/null 2>&1 &
      else
         LOG_OUT "Updated Geoip Database No Change, Do Nothing..."
         sleep 5
      fi
   else
      LOG_OUT "Geoip Database Update Error, Please Try Again Later..."
      sleep 5
   fi
   rm -rf /tmp/Country.mmdb >/dev/null 2>&1
   SLOG_CLEAN
   del_lock