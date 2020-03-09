#!/usr/bin/env bash

base_dir=`dirname $0`
source "$base_dir/utils.sh"
config="$base_dir/config.json"

echoToLogreader() {
    logger -t CloudDisk "${1}"
}

accessToken=`getSingleJsonValue "$config" "accessToken"`
method=`getSingleJsonValue "$config" "method"`
UA=`getSingleJsonValue "$config" "User-Agent"`
extra_header="User-Agent:$UA"


HOST="http://api.cloud.189.cn"
LOGIN_URL="/loginByOpen189AccessToken.action"
ACCESS_URL="/speed/startSpeedV2.action"
echoToLogreader "*******************************************"
echoToLogreader "Sending Heartbeat Package ..."
split="~"
headers_string="$extra_header"
headers=`formatHeaderString "$split" "$headers_string"`
result=`get "$HOST$LOGIN_URL?accessToken=$accessToken" "$headers"`
session_key=`echo "$result" | grep -Eo "sessionKey>.*</sessionKey" | sed 's/<\/sessionKey//' | sed 's/sessionKey>//'`
session_secret=`echo "$result" | grep -Eo "sessionSecret>.*</sessionSecret" | sed 's/sessionSecret>//' | sed 's/<\/sessionSecret//'`
date=`env LANG=C.UTF-8 date -u '+%a, %d %b %Y %T GMT'`
data="SessionKey=$session_key&Operate=$method&RequestURI=$ACCESS_URL&Date=$date"
key="$session_secret"
signature=`hashHmac "sha1" "$data" "$key"`
headers_string="SessionKey:$session_key"${split}"Signature:$signature"${split}"Date:$date"${split}"$extra_header"
headers=`formatHeaderString "$split" "$headers_string"`
qosClientSn=`cat /proc/sys/kernel/random/uuid`
result=`get "$HOST$ACCESS_URL?qosClientSn=$qosClientSn" "$headers"`
echoToLogreader "Heartbeat Signature: $signature"
echoToLogreader "Date: $date"
echoToLogreader "Response: $result"
[[ "`echo ${result} | grep dialAcc`" != "" ]] &&  hint="succeeded" || hint="failed"
echoToLogreader "Heartbeating $hint."
echoToLogreader "*******************************************"
