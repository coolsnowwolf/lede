#!/bin/sh
entry_num=0
if [ "$1" != "" ]; then
entry_num=$1
fi

key_base64=$(uci -q get fwknopd.@access[$entry_num].KEY_BASE64)
key=$(uci -q get fwknopd.@access[$entry_num].KEY)
hmac_key_base64=$(uci -q get fwknopd.@access[$entry_num].HMAC_KEY_BASE64)
hmac_key=$(uci -q get fwknopd.@access[$entry_num].HMAC_KEY)

if [ "$key_base64" != "" ]; then
qr="KEY_BASE64:$key_base64"
fi
if [ "$key" != "" ]; then
qr="$qr KEY:$key"

fi
if [ "$hmac_key_base64" != "" ]; then
qr="$qr HMAC_KEY_BASE64:$hmac_key_base64"
fi
if [ "$hmac_key" != "" ]; then
qr="$qr HMAC_KEY:$hmac_key"
fi

qrencode -t svg -I -o - "$qr"
