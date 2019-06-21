#!/bin/ash
# Author: Anton Chen <contact@antonchen.com>
# Create Date: 2019-05-07 11:26:42
# Last Modified: 2019-05-23 14:03:39
# Description: 
rm -rf /tmp/GeoLite2-Country*
curl -s -k --connect-timeout 30 -m 30 -A "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.132 Safari/537.36" http://geolite.maxmind.com/download/geoip/database/GeoLite2-Country.tar.gz -o /tmp/GeoLite2-Country.tar.gz
cd /tmp
gzip -d /tmp/GeoLite2-Country.tar.gz
tar xf /tmp/GeoLite2-Country.tar
cp -f GeoLite2-Country_*/GeoLite2-Country.mmdb /etc/clash/Country.mmdb
rm -rf /tmp/GeoLite2-Country*
