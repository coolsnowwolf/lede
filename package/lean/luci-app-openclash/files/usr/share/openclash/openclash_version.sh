#!/bin/sh
CKTIME=$(date "+%Y-%m-%d-%H")
LAST_OPVER="/tmp/openclash_last_version"
OP_CV=$(sed -n 1p /etc/openclash/openclash_version 2>/dev/null |awk -F '-' '{print $1}' |awk -F '.' '{print $2$3}')
OP_LV=$(sed -n 1p $LAST_OPVER 2>/dev/null |awk -F '-' '{print $1}' |awk -F '.' '{print $2$3}')
HTTP_PORT=$(uci get openclash.config.http_port 2>/dev/null)
PROXY_ADDR=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)

if [ -s "/tmp/openclash.auth" ]; then
   PROXY_AUTH=$(cat /tmp/openclash.auth |awk -F '- ' '{print $2}' |sed -n '1p' 2>/dev/null)
fi

VERSION_URL="https://raw.githubusercontent.com/vernesong/OpenClash/master/version"
if [ "$CKTIME" != "$(grep "CheckTime" $LAST_OPVER 2>/dev/null |awk -F ':' '{print $2}')" ]; then
   if pidof clash >/dev/null; then
      curl -sL --connect-timeout 10 --retry 2 -x http://$PROXY_ADDR:$HTTP_PORT -U "$PROXY_AUTH" "$VERSION_URL" -o $LAST_OPVER >/dev/null 2>&1
   else
      curl -sL --connect-timeout 10 --retry 2 "$VERSION_URL" -o $LAST_OPVER >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "$LAST_OPVER" ]; then
      if [ "$OP_CV" -ge "$OP_LV" ]; then
         sed -i "/^https:/i\CheckTime:${CKTIME}" "$LAST_OPVER" 2>/dev/null
         sed -i '/^https:/,$d' $LAST_OPVER
      elif [ "$OP_CV" -lt "$OP_LV" ]; then
         sed -i "/^https:/i\CheckTime:${CKTIME}" "$LAST_OPVER" 2>/dev/null
         return 2
      fi
   else
      rm -rf "$LAST_OPVER"
   fi
elif [ "$OP_CV" -ge "$OP_LV" ]; then
   sed -i '/^CheckTime:/,$d' $LAST_OPVER
   echo "CheckTime:$CKTIME" >> $LAST_OPVER
elif [ "$OP_CV" -lt "$OP_LV" ]; then
   return 2
fi
