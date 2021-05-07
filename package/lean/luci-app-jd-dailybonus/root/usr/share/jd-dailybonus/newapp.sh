#!/bin/sh
#
# Copyright (C) 2020 luci-app-jd-dailybonus <jerrykuku@qq.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
# 501 下载脚本出错
# 101 没有新版本无需更新
# 0   更新成功

NAME=jd-dailybonus
TEMP_SCRIPT=/tmp/JD_DailyBonus.js
JD_SCRIPT=/usr/share/jd-dailybonus/JD_DailyBonus.js
LOG_FILE=/var/log/jd_dailybonus.log
CRON_FILE=/etc/crontabs/root
usage() {
    cat <<-EOF
		Usage: app.sh [options]
		Valid options are:

		    -a                      Add Cron
		    -n                      Check 
		    -r                      Run Script
		    -u                      Update Script From Server
		    -s                      Save Cookie And Add Cron
		    -w                      Background Run With Wechat Message
		    -h                      Help
EOF
    exit $1
}

# Common functions

uci_get_by_name() {
    local ret=$(uci get $NAME.$1.$2 2>/dev/null)
    echo ${ret:=$3}
}

uci_get_by_type() {
    local ret=$(uci get $NAME.@$1[0].$2 2>/dev/null)
    echo ${ret:=$3}
}

cancel() {
    if [ $# -gt 0 ]; then
        echo "$1"
    fi
    exit 1
}

add_cron() {
    sed -i '/jd-dailybonus/d' $CRON_FILE
    [ $(uci_get_by_type global auto_run 0) -eq 1 ] && echo $(uci_get_by_type global auto_run_time_m)' '$(uci_get_by_type global auto_run_time_h)' * * * sh /usr/share/jd-dailybonus/newapp.sh -w' >>$CRON_FILE
    [ $(uci_get_by_type global auto_update 0) -eq 1 ] && echo '1 '$(uci_get_by_type global auto_update_time)' * * * sh /usr/share/jd-dailybonus/newapp.sh -u' >>$CRON_FILE
    crontab $CRON_FILE
    /etc/init.d/cron restart
}

# Run Script

notify() {
    grep "】:  Cookie失效" ${LOG_FILE} >/dev/null
    if [ $? -eq 0 ]; then
        title="$(date '+%Y年%m月%d日') 京东签到 Cookie 失效"
    else
        title="$(date '+%Y年%m月%d日') 京东签到"
    fi
    desc=$(cat ${LOG_FILE} | grep -E '签到号|签到概览|签到奖励|其他奖励|账号总计|其他总计' | sed 's/$/&\n/g')
    #serverchan
    sckey=$(uci_get_by_type global serverchan)
    if [ ! -z $sckey ]; then
        serverurlflag=$(uci_get_by_type global serverurl)
        serverurl=https://sc.ftqq.com/
        if [ "$serverurlflag" = "sct" ]; then
            serverurl=https://sctapi.ftqq.com/
        fi
        wget-ssl -q --output-document=/dev/null --post-data="text=$title~&desp=$desc" $serverurl$sckey.send
    fi
    
    #Dingding
    dtoken=$(uci_get_by_type global dd_token)
    if [ ! -z $dtoken ]; then
    	DTJ_FILE=/tmp/jd-djson.json
	echo "{\"msgtype\": \"markdown\",\"markdown\": {\"title\":\"${title}\",\"text\":\"${title} <br/> ${desc}\"}}" > ${DTJ_FILE}
    	wget-ssl -q --output-document=/dev/null --header="Content-Type: application/json" --post-file=/tmp/jd-djson.json "https://oapi.dingtalk.com/robot/send?access_token=${dtoken}"
    fi

    #telegram
    TG_BOT_TOKEN=$(uci_get_by_type global tg_token)
    TG_USER_ID=$(uci_get_by_type global tg_userid)
    API_URL="https://api.telegram.org/bot${TG_BOT_TOKEN}/sendMessage"
    if [ ! -z $TG_BOT_TOKEN ] && [ ! -z $TG_USER_ID ]; then
        text="*$title*
        
\`\`\`
"$desc"
===============================
本消息来自京东签到插件 jd-dailybonus
\`\`\`"
        wget-ssl -q --output-document=/dev/null --post-data="chat_id=$TG_USER_ID&text=$text&parse_mode=markdownv2" $API_URL
    fi
}

run() {
    echo -e $(date '+%Y-%m-%d %H:%M:%S %A') >$LOG_FILE 2>/dev/null
    [ ! -f "/usr/bin/node" ] && echo -e "未安装node.js,请安装后再试!\nNode.js is not installed, please try again after installation!" >>$LOG_FILE && exit 1
    (cd /usr/share/jd-dailybonus/ && node $JD_SCRIPT >>$LOG_FILE 2>/dev/null && notify &)
}

save() {
    lua /usr/share/jd-dailybonus/gen_cookieset.lua
    add_cron
}

# Update Script From Server
download() {
    REMOTE_SCRIPT=$(uci_get_by_type global remote_url)
    wget-ssl --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36" --no-check-certificate -t 3 -T 10 -q $REMOTE_SCRIPT -O $TEMP_SCRIPT
    return $?
}

get_ver() {
    echo $(cat $1 | sed -n '/更新时间/p' | awk '{for (i=1;i<=NF;i++){if ($i ~/v/) {print $i}}}' | sed 's/v//')
}

check_ver() {
    download
    if [ $? -ne 0 ]; then
        cancel "501"
    else
        echo $(get_ver $TEMP_SCRIPT)
    fi
}

update() {
    download
    if [ $? -ne 0 ]; then
        cancel "501"
    fi
    if [ -e $JD_SCRIPT ]; then
        local_ver=$(get_ver $JD_SCRIPT)
    else
        local_ver=0
    fi
    remote_ver=$(get_ver $TEMP_SCRIPT)
    cp -r $TEMP_SCRIPT $JD_SCRIPT
    uci set jd-dailybonus.@global[0].version=$remote_ver
    uci commit jd-dailybonus
    cancel "0"
}

while getopts ":alnruswh" arg; do
    case "$arg" in
    a)
        add_cron
        exit 0
        ;;
    l)
        notify
        exit 0
        ;;
    n)
        check_ver
        exit 0
        ;;
    r)
        run
        exit 0
        ;;
    u)
        update
        exit 0
        ;;
    s)
        save
        exit 0
        ;;
    w)
        run
        exit 0
        ;;
    h)
        usage 0
        ;;
    esac
done
