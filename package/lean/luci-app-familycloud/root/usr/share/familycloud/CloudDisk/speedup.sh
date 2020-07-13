#!/usr/bin/env bash

base_dir=`dirname $0`
source "$base_dir/utils.sh"
config="$base_dir/config.json"


accessToken=`getSingleJsonValue "$config" "accessToken"`
method=`getSingleJsonValue "$config" "method"`
rate=`getSingleJsonValue "$config" "rate"`
UA=`getSingleJsonValue "$config" "User-Agent"`
extra_header="User-Agent:$UA"


HOST="http://api.cloud.189.cn"
LOGIN_URL="/loginByOpen189AccessToken.action"
ACCESS_URL="/speed/startSpeedV2.action"
count=0
echo "*******************************************"
while :
do
    count=$((count+1))
    echo "Sending heart_beat package <$count>"
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
    qosClientSn="$session_key"
    result=`get "$HOST$ACCESS_URL?qosClientSn=$qosClientSn" "$headers"`
    echo "heart_beat:<signature:$signature>"
    echo "date:<$date>"
    echo -e "response:\n$result"
    [[ "`echo ${result} | grep dialAcc`" != "" ]] &&  hint="succeeded" || hint="failed"
    echo "Sending heart_beat package <$count> $hint"
    echo "*******************************************"
    sleep ${rate}
done
