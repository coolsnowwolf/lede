#!/bin/bash
. /usr/share/openclash/log.sh
. /lib/functions.sh

urlencode() {
   if [ "$#" -eq 1 ]; then
      echo "$(/usr/share/openclash/openclash_urlencode.lua "$1")"
   fi
}

   RULE_FILE_NAME="$1"
   LOG_FILE="/tmp/openclash.log"
   RELEASE_BRANCH=$(uci -q get openclash.config.release_branch || echo "master")
   github_address_mod=$(uci -q get openclash.config.github_address_mod || echo 0)
   if [ "$1" == "netflix_domains" ]; then
   	  if [ "$github_address_mod" != "0" ]; then
         if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
            DOWNLOAD_PATH="${github_address_mod}gh/vernesong/OpenClash@$RELEASE_BRANCH/luci-app-openclash/root/usr/share/openclash/res/Netflix_Domains.list"
         elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
            DOWNLOAD_PATH="https://raw.fastgit.org/vernesong/OpenClash/$RELEASE_BRANCH/luci-app-openclash/root/usr/share/openclash/res/Netflix_Domains.list"
         else
            DOWNLOAD_PATH="${github_address_mod}https://raw.githubusercontent.com/vernesong/OpenClash/$RELEASE_BRANCH/luci-app-openclash/root/usr/share/openclash/res/Netflix_Domains.list"
         fi
      else
         DOWNLOAD_PATH="https://raw.githubusercontent.com/vernesong/OpenClash/$RELEASE_BRANCH/luci-app-openclash/root/usr/share/openclash/res/Netflix_Domains.list"
      fi
      RULE_FILE_DIR="/usr/share/openclash/res/Netflix_Domains.list"
      RULE_FILE_NAME="Netflix_Domains"
      RULE_TYPE="netflix"
   elif [ "$1" == "disney_domains" ]; then
      if [ "$github_address_mod" != "0" ]; then
         if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
            DOWNLOAD_PATH="${github_address_mod}gh/vernesong/OpenClash@$RELEASE_BRANCH/luci-app-openclash/root/usr/share/openclash/res/Disney_Plus_Domains.list"
         elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
            DOWNLOAD_PATH="https://raw.fastgit.org/vernesong/OpenClash/$RELEASE_BRANCH/luci-app-openclash/root/usr/share/openclash/res/Disney_Plus_Domains.list"
         else
            DOWNLOAD_PATH="${github_address_mod}https://raw.githubusercontent.com/vernesong/OpenClash/$RELEASE_BRANCH/luci-app-openclash/root/usr/share/openclash/res/Disney_Plus_Domains.list"
         fi
      else
         DOWNLOAD_PATH="https://raw.githubusercontent.com/vernesong/OpenClash/$RELEASE_BRANCH/luci-app-openclash/root/usr/share/openclash/res/Disney_Plus_Domains.list"
      fi
      RULE_FILE_DIR="/usr/share/openclash/res/Disney_Plus_Domains.list"
      RULE_FILE_NAME="Disney_Plus_Domains"
      RULE_TYPE="disney"
   elif [ -z "$(grep "$RULE_FILE_NAME" /usr/share/openclash/res/rule_providers.list 2>/dev/null)" ]; then
      DOWNLOAD_PATH=$(grep -F "$RULE_FILE_NAME" /usr/share/openclash/res/game_rules.list |awk -F ',' '{print $2}' 2>/dev/null)
      RULE_FILE_DIR="/etc/openclash/game_rules/$RULE_FILE_NAME"
      RULE_TYPE="game"
   else
      DOWNLOAD_PATH=$(echo "$RULE_FILE_NAME" |awk -F ',' '{print $1$2}' 2>/dev/null)
      RULE_FILE_NAME=$(grep -F "$RULE_FILE_NAME" /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $NF}' 2>/dev/null)
      RULE_FILE_DIR="/etc/openclash/rule_provider/$RULE_FILE_NAME"
      RULE_TYPE="provider"
   fi

   if [ -z "$DOWNLOAD_PATH" ]; then
      LOG_OUT "Rule File【$RULE_FILE_NAME】Download Error!" && SLOG_CLEAN
      exit 0
   fi

   TMP_RULE_DIR="/tmp/$RULE_FILE_NAME"
   TMP_RULE_DIR_TMP="/tmp/$RULE_FILE_NAME.tmp"
   [ "$RULE_TYPE" != "netflix" ] && [ "$RULE_TYPE" != "disney" ] && DOWNLOAD_PATH=$(urlencode "$DOWNLOAD_PATH")
   
   if [ "$RULE_TYPE" = "netflix" ]; then
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   elif [ "$RULE_TYPE" = "disney" ]; then
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   elif [ "$RULE_TYPE" = "game" ]; then
      if [ "$github_address_mod" != "0" ]; then
         if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"gh/FQrabbit/SSTap-Rule@master/rules/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"FQrabbit/SSTap-Rule/master/rules/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         else
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"https://raw.githubusercontent.com/FQrabbit/SSTap-Rule/master/rules/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         fi
      else
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.githubusercontent.com/FQrabbit/SSTap-Rule/master/rules/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
   elif [ "$RULE_TYPE" = "provider" ]; then
      if [ "$github_address_mod" != "0" ]; then
         if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"gh/"$(echo "$DOWNLOAD_PATH" |awk -F '/master' '{print $1}' 2>/dev/null)"@master"$(echo "$DOWNLOAD_PATH" |awk -F 'master' '{print $2}')" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod""$(echo "$DOWNLOAD_PATH" |awk -F '/master' '{print $1}' 2>/dev/null)"/master"$(echo "$DOWNLOAD_PATH" |awk -F 'master' '{print $2}')" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         else
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"https://raw.githubusercontent.com/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         fi
      else
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.githubusercontent.com/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$TMP_RULE_DIR" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
   fi

   if [ "${PIPESTATUS[0]}" -eq 0 ] && [ -s "$TMP_RULE_DIR" ] && [ -z "$(grep "404: Not Found" "$TMP_RULE_DIR")" ] && [ -z "$(grep "Package size exceeded the configured limit" "$TMP_RULE_DIR")" ]; then
      if [ "$RULE_TYPE" = "game" ]; then
      	cat "$TMP_RULE_DIR" |sed '/^#/d' 2>/dev/null |sed '/^ *$/d' 2>/dev/null |awk '{print "  - "$0}' > "$TMP_RULE_DIR_TMP" 2>/dev/null
      	sed -i '1i\payload:' "$TMP_RULE_DIR_TMP" 2>/dev/null
      	cmp -s "$TMP_RULE_DIR_TMP" "$RULE_FILE_DIR"
      else
         cmp -s "$TMP_RULE_DIR" "$RULE_FILE_DIR"
      fi
         if [ "$?" -ne "0" ]; then
            if [ "$RULE_TYPE" = "game" ]; then
               mv "$TMP_RULE_DIR_TMP" "$RULE_FILE_DIR" >/dev/null 2>&1
            else
               mv "$TMP_RULE_DIR" "$RULE_FILE_DIR" >/dev/null 2>&1
            fi
            rm -rf "$TMP_RULE_DIR" >/dev/null 2>&1
            LOG_OUT "Rule File【$RULE_FILE_NAME】Download Successful!" && SLOG_CLEAN
            exit 1
         else
            LOG_OUT "Rule File【$RULE_FILE_NAME】No Change, Do Nothing!" && SLOG_CLEAN
            rm -rf "$TMP_RULE_DIR" >/dev/null 2>&1
            rm -rf "$TMP_RULE_DIR_TMP" >/dev/null 2>&1
            exit 2
         fi
   else
      rm -rf "$TMP_RULE_DIR" >/dev/null 2>&1
      LOG_OUT "Rule File【$RULE_FILE_NAME】Download Error!" && SLOG_CLEAN
      exit 0
   fi
