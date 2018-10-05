#!/bin/sh

chnroute_data=$(wget -O- -t 3 -T 3 http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest)
[ $? -eq 0 ] && {
    echo "$chnroute_data" | grep ipv4 | grep CN | awk -F\| '{ printf("%s/%d\n", $4, 32-log($5)/log(2)) }' > /tmp/china_ssr.txt
}

if [ -s "/tmp/china_ssr.txt" ];then
  if ( ! cmp -s /tmp/china_ssr.txt /etc/china_ssr.txt );then
    mv /tmp/china_ssr.txt /etc/china_ssr.txt
  fi
fi

wget-ssl --no-check-certificate https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt -O /tmp/gfw.b64
/usr/bin/ssr-gfw

if [ -s "/tmp/gfwnew.txt" ];then
  if ( ! cmp -s /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf );then
    mv /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf
    echo "copy"
  fi
fi

/etc/init.d/shadowsocksr restart