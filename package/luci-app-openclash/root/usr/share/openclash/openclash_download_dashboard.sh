#!/bin/bash
. /usr/share/openclash/log.sh
. /lib/functions.sh

   DASH_NAME="$1"
   DASH_TYPE="$2"
   DASH_FILE_DIR="/tmp/dash.zip"
   DASH_FILE_TMP="/tmp/dash/"
   LOG_FILE="/tmp/openclash.log"
   github_address_mod=$(uci -q get openclash.config.github_address_mod || echo 0)
   if [ "$DASH_NAME" == "Dashboard" ]; then
      UNPACK_FILE_DIR="/usr/share/openclash/ui/dashboard/"
      BACKUP_FILE_DIR="/usr/share/openclash/ui/dashboard_backup/"
		if [ "$DASH_TYPE" == "Official" ]; then
			DOWNLOAD_PATH="https://codeload.github.com/Dreamacro/clash-dashboard/zip/refs/heads/gh-pages"
         FILE_PATH_INCLUDE="clash-dashboard-gh-pages"
      else
			DOWNLOAD_PATH="https://codeload.github.com/MetaCubeX/Razord-meta/zip/refs/heads/gh-pages"
         FILE_PATH_INCLUDE="Razord-meta-gh-pages"
      fi
	else
      UNPACK_FILE_DIR="/usr/share/openclash/ui/yacd/"
      BACKUP_FILE_DIR="/usr/share/openclash/ui/yacd_backup/"
		if [ "$DASH_TYPE" == "Official" ]; then
			DOWNLOAD_PATH="https://codeload.github.com/haishanh/yacd/zip/refs/heads/gh-pages"
         FILE_PATH_INCLUDE="yacd-gh-pages"
      else
			DOWNLOAD_PATH="https://codeload.github.com/MetaCubeX/Yacd-meta/zip/refs/heads/gh-pages"
         FILE_PATH_INCLUDE="Yacd-meta-gh-pages"
      fi
	fi
   
   curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$DOWNLOAD_PATH" -o "$DASH_FILE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$DASH_FILE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"

   if [ "${PIPESTATUS[0]}" -eq 0 ] && [ -s "$DASH_FILE_DIR" ] && [ -z "$(grep "404: Not Found" "$DASH_FILE_DIR")" ] && [ -z "$(grep "Package size exceeded the configured limit" "$DASH_FILE_DIR")" ]; then
      unzip -qt "$DASH_FILE_DIR" >/dev/null 2>&1
      if [ "$?" -eq "0" ]; then
         cp -rf  "$UNPACK_FILE_DIR".  "$BACKUP_FILE_DIR" >/dev/null 2>&1
         rm -rf "$UNPACK_FILE_DIR" >/dev/null 2>&1
         unzip -q "$DASH_FILE_DIR" -d "$DASH_FILE_TMP" >/dev/null 2>&1
         if [ "$?" -eq "0" ] && [ -d "$DASH_FILE_TMP$FILE_PATH_INCLUDE" ]; then
            cp -rf "$DASH_FILE_TMP$FILE_PATH_INCLUDE"/. "$UNPACK_FILE_DIR" >/dev/null 2>&1
            rm -rf "$DASH_FILE_DIR" >/dev/null 2>&1
            rm -rf "$BACKUP_FILE_DIR" >/dev/null 2>&1
            rm -rf "$DASH_FILE_TMP" >/dev/null 2>&1
            LOG_OUT "Control Panel【$DASH_NAME - $DASH_TYPE】Download Successful!" && SLOG_CLEAN
            exit 1
         else
            LOG_OUT "Control Panel【$DASH_NAME - $DASH_TYPE】Unzip Error!" && SLOG_CLEAN
            cp -rf  "$BACKUP_FILE_DIR".  "$UNPACK_FILE_DIR" >/dev/null 2>&1
            rm -rf "$DASH_FILE_DIR" >/dev/null 2>&1
            rm -rf "$BACKUP_FILE_DIR" >/dev/null 2>&1
            rm -rf "$DASH_FILE_TMP" >/dev/null 2>&1
            exit 2
         fi
      else
         LOG_OUT "Control Panel【$DASH_NAME - $DASH_TYPE】Unzip Error!" && SLOG_CLEAN
         cp -rf  "$BACKUP_FILE_DIR".  "$UNPACK_FILE_DIR" >/dev/null 2>&1
         rm -rf "$DASH_FILE_DIR" >/dev/null 2>&1
         rm -rf "$BACKUP_FILE_DIR" >/dev/null 2>&1
         rm -rf "$DASH_FILE_TMP" >/dev/null 2>&1
         exit 2
      fi
   else
      cp -rf  "$BACKUP_FILE_DIR".  "$UNPACK_FILE_DIR" >/dev/null 2>&1
      rm -rf "$BACKUP_FILE_DIR" >/dev/null 2>&1
      rm -rf "$DASH_FILE_DIR" >/dev/null 2>&1
      rm -rf "$DASH_FILE_TMP" >/dev/null 2>&1
      LOG_OUT "Control Panel【$DASH_NAME - $DASH_TYPE】Download Error!" && SLOG_CLEAN
      exit 0
   fi
