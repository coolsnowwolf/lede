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
LOG_HTM=/www/JD_DailyBonus.htm
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

REMOTE_SCRIPT=$(uci_get_by_type global remote_url)

fill_cookie() {
    cookie1=$(uci_get_by_type global cookie)
    if [ ! "$cookie1" = "" ]; then
        varb="var Key = '$cookie1';"
        a=$(sed -n '/var Key =/=' $JD_SCRIPT)
        b=$((a-1))
        sed -i "${a}d" $JD_SCRIPT
        sed -i "${b}a ${varb}" $JD_SCRIPT
    fi

    cookie2=$(uci_get_by_type global cookie2)
    if [ ! "$cookie2" = "" ]; then
        varb2="var DualKey = '$cookie2';"
        aa=$(sed -n '/var DualKey =/=' $JD_SCRIPT)
        bb=$((aa-1))
        sed -i "${aa}d" $JD_SCRIPT
        sed -i "${bb}a ${varb2}" $JD_SCRIPT
    fi

    stop=$(uci_get_by_type global stop)
    if [ ! "$stop" = "" ]; then
        varb3="var stop = $stop;"
        sed -i "s/^var stop =.*/$varb3/g" $JD_SCRIPT
    fi
}

if [ -e $TEMP_SCRIPT ]; then
    remote_ver=$(cat $TEMP_SCRIPT | sed -n '/更新时间/p' | awk '{for (i=1;i<=NF;i++){if ($i ~/v/) {print $i}}}' | sed 's/v//')
else
    remote_ver=$(cat $JD_SCRIPT | sed -n '/更新时间/p' | awk '{for (i=1;i<=NF;i++){if ($i ~/v/) {print $i}}}' | sed 's/v//')
fi
local_ver=$(uci_get_by_type global version)

add_cron() {
    sed -i '/jd-dailybonus/d' $CRON_FILE
    [ $(uci_get_by_type global auto_run 0) -eq 1 ] && echo '5 '$(uci_get_by_type global auto_run_time)' * * * /bin/bash -c "sleep $[RANDOM % 180]s"; /usr/share/jd-dailybonus/newapp.sh -w' >>$CRON_FILE
    [ $(uci_get_by_type global auto_update 0) -eq 1 ] && echo '1 '$(uci_get_by_type global auto_update_time)' * * * /usr/share/jd-dailybonus/newapp.sh -u' >>$CRON_FILE
    crontab $CRON_FILE
}

# Run Script

serverchan() {
    sckey=$(uci_get_by_type global serverchan)
    failed=$(uci_get_by_type global failed)
    desc=$(cat /www/JD_DailyBonus.htm | sed 's/$/&\n/g' | sed -e '/左滑/d')
    serverurlflag=$(uci_get_by_type global serverurl)
    serverurl=https://sc.ftqq.com/
    if [ "$serverurlflag" = "sct" ]; then
        serverurl=https://sctapi.ftqq.com/
    fi
    if [ $failed -eq 1 ]; then
        grep "Cookie失效" /www/JD_DailyBonus.htm > /dev/null
        if [ $? -eq 0 ]; then
            title="$(date '+%Y年%m月%d日') 京东签到 Cookie 失效"
            wget-ssl -q --output-document=/dev/null --post-data="text=$title~&desp=$desc" $serverurl$sckey.send
        fi
    else
        title="$(date '+%Y年%m月%d日') 京东签到"
        wget-ssl -q --output-document=/dev/null --post-data="text=$title~&desp=$desc" $serverurl$sckey.send
    fi

}

run() {
    fill_cookie
    echo -e $(date '+%Y-%m-%d %H:%M:%S %A') >$LOG_HTM 2>/dev/null
    node $JD_SCRIPT >>$LOG_HTM 2>&1 &
}

back_run() {
    fill_cookie
    echo -e $(date '+%Y-%m-%d %H:%M:%S %A') >$LOG_HTM 2>/dev/null
    node $JD_SCRIPT >>$LOG_HTM 2>/dev/null
    serverchan
}

save() {
    fill_cookie
    add_cron
}

# Update Script From Server

check_ver() {
    wget-ssl --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36" --no-check-certificate -t 3 -T 10 -q $REMOTE_SCRIPT -O $TEMP_SCRIPT
    if [ $? -ne 0 ]; then
        cancel "501"
    else
        echo $remote_ver
    fi
}

update() {
    wget-ssl --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36" --no-check-certificate -t 3 -T 10 -q $REMOTE_SCRIPT -O $TEMP_SCRIPT
    if [ $? -ne 0 ]; then
        cancel "501"
    fi
    if [ $(expr $local_ver \< $remote_ver) -eq 1 ]; then
        cp -r $TEMP_SCRIPT $JD_SCRIPT
        fill_cookie
        uci set jd-dailybonus.@global[0].version=$remote_ver
        uci commit jd-dailybonus
        cancel "0"
    else
        cancel "101"
    fi
}

while getopts ":anruswh" arg; do
    case "$arg" in
    a)
        add_cron
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
        back_run
        exit 0
        ;;
    h)
        usage 0
        ;;
    esac
done
