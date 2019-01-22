#!/bin/sh

LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")

echo '['$LOGTIME'] 正在下载chinaDNS更新文件.'
chnroute_data=$(wget -O- -t 3 -T 3 http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest)
[ $? -eq 0 ] && {
    echo "$chnroute_data" | grep ipv4 | grep CN | awk -F\| '{ printf("%s/%d\n", $4, 32-log($5)/log(2)) }' > /tmp/china_ssr.txt
}

if [ -s "/tmp/china_ssr.txt" ];then
	echo '['$LOGTIME'] chinaDNS更新文件下载完成.'
	if ( ! cmp -s /tmp/china_ssr.txt /etc/china_ssr.txt );then
	echo '['$LOGTIME'] 检测到chinaDNS有更新正在写入中.'
    mv /tmp/china_ssr.txt /etc/china_ssr.txt
	echo '['$LOGTIME'] 写入完成.'
  fi
fi

echo '['$LOGTIME'] 正在下载GFW更新文件.'
wget-ssl --no-check-certificate https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt -O /tmp/gfw.b64
/usr/bin/ssr-gfw

if [ -s "/tmp/gfwnew.txt" ];then
	echo '['$LOGTIME'] GFW更新文件下载完成.'
	if ( ! cmp -s /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf );then
	echo '['$LOGTIME'] 检测到GFW有更新正在写入中.'
    mv /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf
	echo '['$LOGTIME'] 写入完成.'
  fi
fi

echo '['$LOGTIME'] 规则更新完成,重启shadowsocksr.'
/etc/init.d/shadowsocksr restart
