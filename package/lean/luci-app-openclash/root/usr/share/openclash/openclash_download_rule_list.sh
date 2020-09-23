#!/bin/sh
   RULE_FILE_NAME="$1"
   if [ -z "$(grep "$RULE_FILE_NAME" /usr/share/openclash/res/rule_providers.list 2>/dev/null)" ]; then
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
      echo "${LOGTIME} Rule File【$RULE_FILE_NAME】 Download Error" >>$LOG_FILE
      return 0
   fi

   TMP_RULE_DIR="/tmp/$RULE_FILE_NAME"
   LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
   LOG_FILE="/tmp/openclash.log"
   HTTP_PORT=$(uci get openclash.config.http_port 2>/dev/null)
   PROXY_ADDR=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)
   if [ -s "/tmp/openclash.auth" ]; then
      PROXY_AUTH=$(cat /tmp/openclash.auth |awk -F '- ' '{print $2}' |sed -n '1p' 2>/dev/null)
   fi
   if [ "$RULE_TYPE" = "game" ]; then
      if pidof clash >/dev/null; then
   	     curl -sL --connect-timeout 5 --retry 2 -x http://$PROXY_ADDR:$HTTP_PORT -U "$PROXY_AUTH" https://raw.githubusercontent.com/FQrabbit/SSTap-Rule/master/rules/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" >/dev/null 2>&1
      else
         curl -sL --connect-timeout 5 --retry 2 https://cdn.jsdelivr.net/gh/FQrabbit/SSTap-Rule@master/rules/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" >/dev/null 2>&1
      fi
   elif [ "$RULE_TYPE" = "provider" ]; then
      if pidof clash >/dev/null; then
   	     curl -sL --connect-timeout 5 --retry 2 -x http://$PROXY_ADDR:$HTTP_PORT -U "$PROXY_AUTH" https://raw.githubusercontent.com/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" >/dev/null 2>&1
      else
         curl -sL --connect-timeout 5 --retry 2 https://cdn.jsdelivr.net/gh/"$(echo "$DOWNLOAD_PATH" |awk -F '/master' '{print $1}' 2>/dev/null)"@master"$(echo "$DOWNLOAD_PATH" |awk -F 'master' '{print $2}')" -o "$TMP_RULE_DIR" >/dev/null 2>&1
      fi
   fi

   if [ "$?" -eq "0" ] && [ -s "$TMP_RULE_DIR" ] && [ -z "$(grep "404: Not Found" "$TMP_RULE_DIR")" ]; then
      cmp -s "$TMP_RULE_DIR" "$RULE_FILE_DIR"
         if [ "$?" -ne "0" ]; then
            mv "$TMP_RULE_DIR" "$RULE_FILE_DIR" >/dev/null 2>&1\
            && rm -rf "$TMP_RULE_DIR" >/dev/null 2>&1
            echo "${LOGTIME} Rule File【$RULE_FILE_NAME】 Download Successful" >>$LOG_FILE
            return 1
         else
            echo "${LOGTIME} Updated Rule File【$RULE_FILE_NAME】 No Change, Do Nothing" >>$LOG_FILE
            rm -rf "$TMP_RULE_DIR" >/dev/null 2>&1
            return 2
         fi
   else
      rm -rf "$TMP_RULE_DIR" >/dev/null 2>&1
      echo "${LOGTIME} Rule File【$RULE_FILE_NAME】 Download Error" >>$LOG_FILE
      return 0
   fi