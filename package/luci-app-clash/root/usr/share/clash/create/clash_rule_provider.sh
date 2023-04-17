#!/bin/sh

   RULE_FILE_NAME="$1" 

   if [ -z "$(grep ",$RULE_FILE_NAME" /usr/share/clash/create/rule_provider.list 2>/dev/null)" ]; then
      DOWNLOAD_PATH=$(grep -F "$RULE_FILE_NAME" /usr/share/clash/create/rule_provider.list |awk -F ',' '{print $3$4}' 2>/dev/null)
   else
      DOWNLOAD_PATH=$(grep -F ",$RULE_FILE_NAME" /usr/share/clash/create/rule_provider.list |awk -F ',' '{print $3$4}' 2>/dev/null)
   fi
   
   RULE_FILE_DIR="/etc/clash/ruleprovider/$RULE_FILE_NAME"
   TMP_RULE_DIR="/tmp/$RULE_FILE_NAME"
   LOG_FILE="/usr/share/clash/clash.txt"
   REAL_LOG="/usr/share/clash/clash_real.txt"
   
   url="https://cdn.jsdelivr.net/gh/$DOWNLOAD_PATH"
   behavior=$(grep -F $RULE_FILE_NAME /usr/share/clash/create/rule_provider.list |awk -F ',' '{print $2}' 2>/dev/null)
   namee=$(grep -F ",$RULE_FILE_NAME" /usr/share/clash/create/rule_provider.list |awk -F ',' '{print $4}' | awk -F '.' '{print $1}' 2>/dev/null)
   path="./ruleprovider/"$RULE_FILE_NAME""
   
   curl -sL --connect-timeout 5 --retry 2 https://cdn.jsdelivr.net/gh/"$DOWNLOAD_PATH" -o "$TMP_RULE_DIR" >/dev/null 2>&1
   	
   if [ "$?" -eq "0" ] && [ -s "$TMP_RULE_DIR" ] && [ -z "$(grep "404: Not Found" "$TMP_RULE_DIR")" ]; then
      cmp -s $TMP_RULE_DIR $RULE_FILE_DIR
         if [ "$?" -ne "0" ]; then
            mv $TMP_RULE_DIR $RULE_FILE_DIR >/dev/null 2>&1\
            && rm -rf $TMP_RULE_DIR >/dev/null 2>&1
			   name=clash
			   uci_name_tmp=$(uci add $name ruleprovider)
			   uci_set="uci -q set $name.$uci_name_tmp."
			   uci_add="uci -q add_list $name.$uci_name_tmp."
			   ${uci_set}type="http"   
			   ${uci_set}name="$namee"
			   ${uci_set}path="$path"
			   ${uci_set}behavior="$behavior"
			   ${uci_set}url="$url"
			   ${uci_set}interval="86400"
			   uci commit clash
            return 1
	
         else
            rm -rf $TMP_RULE_DIR >/dev/null 2>&1
            return 2
         fi
   else
      rm -rf $TMP_RULE_DIR >/dev/null 2>&1
      return 0
   fi
