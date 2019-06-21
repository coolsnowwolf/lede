#!/bin/sh
# Author: Anton Chen <contact@antonchen.com>
# Create Date: 2019-05-06 15:04:48
# Last Modified: 2019-05-28 11:17:46
# Description: 
. /lib/functions/network.sh
CONF="/etc/clash/config.yml"
CONF_TEMP="/tmp/clash-temp.yml"
CONF_TEMPLATE="/etc/clash/config.tpl"
GEN_SUBSCRIPTION="/usr/share/clash/subscription-to-clash.lua"
CUSTOM_PROXY="/etc/clash/custom-proxy"
CUSTOM_GROUP="/etc/clash/custom-group"
CUSTOM_SELECT="/etc/clash/custom-select"
CUSTOM_RULE="/etc/clash/custom-rule"
SUBSCRIPTION_PROXY="/etc/clash/subscription-proxy"
SUBSCRIPTION_GROUP="/etc/clash/subscription-group"

# remove config temp file
test -f $CONF_TEMP && rm -f $CONF_TEMP

splitString ()
{
    local list
    echo $1|grep -q ','
    if [ $? -eq 0 ]; then
        list=$(echo $1|sed -n 's/,/ /gp')
    else
        list=$1
    fi
    echo $list
}

lineCount ()
{
    local FILE
    FILE=$1
    if [ ! -f $FILE ]; then
        return 0
    fi
    return $(egrep -v '^#|^$' $FILE|wc -l)
}

if [ $(uci get clash.@general[0].enabled 2>/dev/null) -eq 1 ] && [ $(uci get clash.@subscription[0].enabled 2>/dev/null) -eq 1 ]; then
    lua $GEN_SUBSCRIPTION
fi

# if proxy is null, exit
lineCount $CUSTOM_PROXY
RC_C=$?
lineCount $SUBSCRIPTION_PROXY
RC_S=$?
if [ $RC_C -eq 0 ] && [ $RC_S -eq 0 ]; then
    exit
fi

HTTP_PORT=$(uci get clash.@general[0].http_port 2>/dev/null)
if [ ! -z $HTTP_PORT ]; then
    echo "port: $HTTP_PORT" >> $CONF_TEMP
fi
SOCKS_PORT=$(uci get clash.@general[0].socks_port 2>/dev/null)
if [ ! -z $SOCKS_PORT ]; then
    echo "socks-port: $SOCKS_PORT" >> $CONF_TEMP
fi
REDIR_PORT=$(uci get clash.@general[0].redir_port 2>/dev/null)
[ -z $REDIR_PORT ] && exit
echo "redir-port: $REDIR_PORT" >> $CONF_TEMP
echo 'mode: Rule' >> $CONF_TEMP
LOG_LEVEL=$(uci get clash.@general[0].log_level 2>/dev/null)
if [ -z $LOG_LEVEL ]; then
    echo "log-level: warning" >> $CONF_TEMP
else
    echo "log-level: $LOG_LEVEL" >> $CONF_TEMP
fi
API_PORT=$(uci get clash.@general[0].api_port 2>/dev/null)
if [ ! -z $API_PORT ]; then
    network_get_ipaddr LAN_ADDR lan
    if [ -z $LAN_ADDR ]; then
        echo "external-controller: 0.0.0.0:$API_PORT" >> $CONF_TEMP
    else
        echo "external-controller: $LAN_ADDR:$API_PORT" >> $CONF_TEMP
    fi
fi
SECRET=$(uci get clash.@general[0].secret 2>/dev/null)
[ -z $SECRET ] && exit
echo "secret: $SECRET" >> $CONF_TEMP

echo 'allow-lan: true' >> $CONF_TEMP

# Check config template
TPL_URL=$(uci get clash.@general[0].tpl_url 2>/dev/null)
[ $? -ne 0 ] && exit
updateTemplate ()
{
    if [ ! -f $CONF_TEMPLATE ]; then
        curl -s -k --connect-timeout 5 -m 5 -A "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.132 Safari/537.36" -o $CONF_TEMPLATE $TPL_URL || exit
        grep -q '# Rule-END' $CONF_TEMPLATE
        if [ $? -ne 0 ]; then
            rm -f $CONF_TEMPLATE
            exit 1
        fi
    fi
}
updateTemplate
if [ $(date +%Y%m%d -r $CONF_TEMPLATE) -le $(date +%Y%m%d -d "@$(( $(busybox date +%s) - 86400 * 7 ))") ]; then
    rm -f $CONF_TEMPLATE
    updateTemplate
fi

# DNS
DNS_PORT=$(uci get clash.@general[0].dns_port 2>/dev/null)
[ $? -ne 0 ] && exit
DNS_TPL=$(awk '/# DNS-START/,/# DNS-END/{if(i>1)print x;x=$0;i++}' $CONF_TEMPLATE)
[ -z "$DNS_TPL" ] && exit
cat >> $CONF_TEMP << _DNS_
$(echo "$DNS_TPL"|sed "s/listen:.*\$/listen: 127.0.0.1:$DNS_PORT/g")
_DNS_

# Set Proxy
echo 'Proxy:' >> $CONF_TEMP
if [ $RC_C -ne 0 ]; then
    cat $CUSTOM_PROXY >> $CONF_TEMP
    echo >> $CONF_TEMP
fi
if [ $RC_S -ne 0 ] && [ $(uci get clash.@subscription[0].enabled 2>/dev/null) -eq 1 ]; then
    cat $SUBSCRIPTION_PROXY >> $CONF_TEMP
fi
awk '/# Proxy-START/,/# Proxy-END/{if(i>1)print x;x=$0;i++}' $CONF_TEMPLATE|sed '/Proxy:/d' >> $CONF_TEMP

# Set Group
echo 'Proxy Group:' >> $CONF_TEMP
lineCount $SUBSCRIPTION_GROUP
if [ $? -ne 0 ] && [ $(uci get clash.@subscription[0].enabled 2>/dev/null) -eq 1 ]; then
    cat $SUBSCRIPTION_GROUP >> $CONF_TEMP
fi
lineCount $CUSTOM_GROUP
if [ $? -ne 0 ]; then
    cat $CUSTOM_GROUP >> $CONF_TEMP
    echo >> $CONF_TEMP
fi
awk '/# Group-START/,/# Group-END/{if(i>1)print x;x=$0;i++}' $CONF_TEMPLATE|sed '/^Proxy Group:/d' >> $CONF_TEMP

# Set Select
lineCount $CUSTOM_SELECT
if [ $? -ne 0 ]; then
    cat $CUSTOM_SELECT >> $CONF_TEMP
    echo >> $CONF_TEMP
fi
awk '/# Select-START/,/# Select-END/{if(i>1)print x;x=$0;i++}' $CONF_TEMPLATE >> $CONF_TEMP

# Set Rule
echo 'Rule:' >> $CONF_TEMP
lineCount $CUSTOM_RULE
if [ $? -ne 0 ]; then
    cat $CUSTOM_RULE >> $CONF_TEMP
    echo >> $CONF_TEMP
fi
awk '/# Rule-START/,/# Rule-END/{if(i>1)print x;x=$0;i++}' $CONF_TEMPLATE|sed '/^Rule:/d' >> $CONF_TEMP

cat $CONF_TEMP > $CONF
