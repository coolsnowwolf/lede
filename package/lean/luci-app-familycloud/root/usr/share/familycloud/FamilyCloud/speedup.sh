#!/usr/bin/env bash

base_dir=`dirname $0`
source "$base_dir/utils.sh"
config="$base_dir/config.json"


accessToken=`getSingleJsonValue "$config" "accessToken"`
AppKey=`getSingleJsonValue "$config" "AppKey"`
method=`getSingleJsonValue "$config" "method"`
rate=`getSingleJsonValue "$config" "rate"`
prodCode=`getSingleJsonValue "$config" "prodCode"`
UA=`getSingleJsonValue "$config" "User-Agent"`
extra_header="User-Agent:$UA"


HOST="http://api.cloud.189.cn"
LOGIN_URL="/login4MergedClient.action"
ACCESS_URL="/family/qos/startQos.action"
count=0
echo "*******************************************"
while :
do
    count=$((count+1))
    echo "Sending heart_beat package <$count>"
    split="~"
    headers_string="AppKey:$AppKey"${split}"$extra_header"
    headers=`formatHeaderString "$split" "$headers_string"`
    result=`post "$headers" "$HOST$LOGIN_URL?accessToken=$accessToken"`
    session_key=`echo "$result" | grep -Eo "familySessionKey>.+</familySessionKey" | sed 's/familySessionKey>//' | sed 's/<\/familySessionKey//'`
    session_secret=`echo "$result" | grep -Eo "familySessionSecret>.+</familySessionSecret" | sed 's/familySessionSecret>//' | sed 's/<\/familySessionSecret//'`
    date=`env LANG=C.UTF-8 date -u '+%a, %d %b %Y %T GMT'`
    data="SessionKey=$session_key&Operate=$method&RequestURI=$ACCESS_URL&Date=$date"
    key="$session_secret"
    signature=`hashHmac "sha1" "$data" "$key"`
    headers_string="SessionKey:$session_key"${split}"Signature:$signature"${split}"Date:$date"${split}"$extra_header"
    headers=`formatHeaderString "$split" "$headers_string"`
    send_data="prodCode=$prodCode"
    result=`post "$headers" "$HOST$ACCESS_URL" "$send_data"`
    echo "heart_beat:<signature:$signature>"
    echo "date:<$date>"
    echo "status_code:${result: -3}"
    echo -e "response:\n`echo ${result} | sed "s^[0-9]\{3\}$^^"`"
    [[ "`echo ${result} | grep dialAcc`" != "" ]] &&  hint="succeeded" || hint="failed"
    echo "Sending heart_beat package <$count> $hint"
    echo "*******************************************"
    sleep ${rate}
done