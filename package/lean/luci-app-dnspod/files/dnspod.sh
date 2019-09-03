#!/bin/bash
#written by Michael Qu
#greennyreborn@gmail.com

#dnspod api token
token=`uci get dnspod.base_arg.login_token`

#域名
domain=`uci get dnspod.base_arg.main_domain 2>/dev/null`


#需要更新的子域名，只允许一个
#由于*linux会被脚本解析为当前目录下的所有文件，所以请填写\*
#  sub_domain='\*'
sub_domain=`uci get dnspod.base_arg.sub_domain 2>/dev/null`
if [ $sub_domain = "\*" ];then
    sub_domain="*"
fi

#check for changed ip every 300 seconds
wait=300
wait_second=`uci get dnspod.base_arg.wait_second 2>/dev/null`
if [[ $wait_second =~ ^[1-9][0-9]*$ ]]; then
    wait=$wait_second
fi

#检查curl是否安装
curl_status=`which curl 2>/dev/null`
[ -n "$curl_status" ] || { echo "curl is not installed";exit 3; }

os=$(uname -a | egrep -io 'openwrt' | tr [A-Z] [a-z])

API_url="https://dnsapi.cn"
LOG_FILE="/tmp/dnspod.log"
MAX_LOG_SIZE=1*1024*1024
PROGRAM=$(basename $0)

format='json'
lang='en'
record_type='A'
common_options="login_token=$token&format=${format}&lang=${lang}"

is_svc=0
last_modified_ip=

getFileSize() {
    local file=$1
    wc -c $file
}

clearLog() {
    local file_size=$(getFileSize $LOG_FILE)
    if [ $file_size -gt $MAX_LOG_SIZE ]; then
        echo "" > $LOG_FILE
    fi

}

printMsg() {
    local time=$(date "+%Y-%m-%d %H:%M:%S")
    local msg="$1"
    if [ $is_svc -eq 1 ];then
        echo "$time: $msg" >> $LOG_FILE
        # logger -t ${PROGRAM} "${msg}"
    else
        echo "$time: $msg"
    fi
}

getIp() {
#    curl -s http://checkip.dyndns.com | sed -n 's/.*: \([0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\).*/\1/p'
    curl -s whatismyip.akamai.com
}

execAPI() {
    local action="$1"
    local extra_options="$2"

    eval "curl -k -X POST \"${API_url}/${action}\" -d \"${common_options}&${extra_options}\""
}

getRecordList() {
    local extra_options="domain=$domain"
    echo $(execAPI "Record.List" "$extra_options")
    exit
}

updateRecord() {
    local ip=$1
    local record_list=`getRecordList`
    local old_ip=`jsonfilter -s "$record_list" -e "@.records[@.name='$sub_domain'].value"`
    local id=`jsonfilter -s "$record_list" -e "@.records[@.name='$sub_domain'].id"`
    printMsg "Change record: $sub_domain.$domain from $old_ip to $ip"

    local extra_options="domain=$domain&record_id=$id&value=$ip&record_type=A&record_line_id=0&sub_domain=$sub_domain"
    local response=$(execAPI "Record.Modify" "$extra_options")

    local code=`jsonfilter -s "$response" -e "@.status.code"`
    if [ "$code" == "1" ]; then
        last_modified_ip=$ip
    fi
    local message=`jsonfilter -s "$response" -e "@.status.message"`
    printMsg "response: $message"

}

checkip() {
    local wan_ip=$1
    printMsg "old ip: [$last_modified_ip], new ip: [$wan_ip]"
    if [ "$wan_ip" != "$last_modified_ip" ];then
        return 8
    else
        return 3
    fi
}

execSvc() {
    local ip
    #check that whether the network is ok
    while [ 1 ];do
        ip=$(getIp)
        if [ -n "$ip" ];then
            printMsg "WAN IP: ${ip}"
            break;
        else
            printMsg "Can't get wan ip"
            sleep 30
        fi
    done

    while [ 1 ];do
        clearLog
        ip=$(getIp)
        checkip $ip
        if [ $? -eq 8 ];then
            updateRecord $ip
            #updateTunnelBroker
        fi
        sleep $wait
    done
}

execUpdate() {
    local ip=`getIp`
    updateRecord $ip
}

case $1 in
    --svc)
        is_svc=1;
        clearLog;
        printMsg "Start in Service mode, check in every $wait seconds";
        printMsg "domain: ${domain}, sub_domain: ${sub_domain}";
        execSvc;;
    *)
        is_svc=0;
        printMsg "Start update record, domain: ${domain}, sub_domain: ${sub_domain}";
        execUpdate;;
esac
