#!/bin/sh

script_action=${1}

logfile_path() (
    configfile=$(uci -q get mosdns.config.configfile)
    if [ "$configfile" = "/etc/mosdns/config.yaml" ]; then
        uci -q get mosdns.config.logfile
    else
        [ ! -f /etc/mosdns/config_custom.yaml ] && exit 1
        awk '/^log:/{f=1;next}f==1{if($0~/file:/){print;exit}if($0~/^[^ ]/)exit}' /etc/mosdns/config_custom.yaml | grep -Eo "/[^'\"]+"
    fi
)

interface_dns() (
    if [ "$(uci -q get mosdns.config.custom_local_dns)" = 1 ]; then
        uci -q get mosdns.config.local_dns
    else
        peerdns=$(uci -q get network.wan.peerdns)
        proto=$(uci -q get network.wan.proto)
        if [ "$peerdns" = 0 ] || [ "$proto" = "static" ]; then
            uci -q get network.wan.dns
        else
            interface_status=$(ubus call network.interface.wan status)
            echo $interface_status | jsonfilter -e "@['dns-server'][0]"
            echo $interface_status | jsonfilter -e "@['dns-server'][1]"
        fi
        [ $? -ne 0 ] && echo "119.29.29.29 223.5.5.5"
    fi
)

get_adlist() (
    adblock=$(uci -q get mosdns.config.adblock)
    if [ "$adblock" = 1 ]; then
        mkdir -p /etc/mosdns/rule/adlist
        ad_source=$(uci -q get mosdns.config.ad_source)
        for url in $ad_source;
        do
            if [ $(echo $url) = 'geosite.dat' ]; then
                echo "        - \"/var/mosdns/geosite_category-ads-all.txt\""
            elif echo "$url" | grep -Eq "^file://" ; then
                echo "        - \"$(echo "$url" | sed 's/file:\/\///')\""
            else
                echo "        - \"/etc/mosdns/rule/adlist/$(basename $url)\""
                [ ! -f "/etc/mosdns/rule/adlist/$(basename $url)" ] && touch /etc/mosdns/rule/adlist/$(basename $url)
            fi
        done
    else
        rm -rf /etc/mosdns/rule/adlist /etc/mosdns/rule/.ad_source
        touch /var/disable-ads.txt
        echo "        - \"/var/disable-ads.txt\""
    fi
)

adlist_update() {
    [ "$(uci -q get mosdns.config.adblock)" != 1 ] && exit 0
    ad_source=$(uci -q get mosdns.config.ad_source)
    AD_TMPDIR=$(mktemp -d) || exit 1
    mirror=""
    : > /etc/mosdns/rule/.ad_source
    has_update=0
    for url in $ad_source;
    do
        if [ "$url" != "geosite.dat" ] && [ $(echo "$url" | grep -c -E "^file://") -eq 0 ]; then
            echo "$url" >> /etc/mosdns/rule/.ad_source
            filename=$(basename $url)
            if echo "$url" | grep -Eq "^https://raw.githubusercontent.com" ; then
                [ -n "$(uci -q get mosdns.config.github_proxy)" ] && mirror="$(uci -q get mosdns.config.github_proxy)/"
            fi
            echo -e "\e[1;32mDownloading $mirror$url\e[0m"
            curl --connect-timeout 5 -m 90 --ipv4 -kfSLo "$AD_TMPDIR/$filename" "$mirror$url"
            has_update=1
        fi
    done
    if [ $? -ne 0 ]; then
        echo -e "\e[1;31mRules download failed.\e[0m"
        rm -rf "$AD_TMPDIR"
        exit 1
    else
        [ $has_update -eq 1 ] && {
            mkdir -p /etc/mosdns/rule/adlist
            rm -rf /etc/mosdns/rule/adlist/*
            \cp $AD_TMPDIR/* /etc/mosdns/rule/adlist
            rm -rf "$AD_TMPDIR"
        }
    fi
}

geodat_update() (
    TMPDIR=$(mktemp -d) || exit 1
    [ -n "$(uci -q get mosdns.config.github_proxy)" ] && mirror="$(uci -q get mosdns.config.github_proxy)/"
    # geoip.dat - cn-private
    echo -e "\e[1;32mDownloading "$mirror"https://github.com/Loyalsoldier/geoip/releases/latest/download/geoip-only-cn-private.dat\e[0m"
    curl --connect-timeout 5 -m 60 --ipv4 -kfSLo "$TMPDIR/geoip.dat" ""$mirror"https://github.com/Loyalsoldier/geoip/releases/latest/download/geoip-only-cn-private.dat"
    [ $? -ne 0 ] && rm -rf "$TMPDIR" && exit 1
    # checksum - geoip.dat
    echo -e "\e[1;32mDownloading "$mirror"https://github.com/Loyalsoldier/geoip/releases/latest/download/geoip-only-cn-private.dat.sha256sum\e[0m"
    curl --connect-timeout 5 -m 10 --ipv4 -kfSLo "$TMPDIR/geoip.dat.sha256sum" ""$mirror"https://github.com/Loyalsoldier/geoip/releases/latest/download/geoip-only-cn-private.dat.sha256sum"
    [ $? -ne 0 ] && rm -rf "$TMPDIR" && exit 1
    if [ "$(sha256sum "$TMPDIR/geoip.dat" | awk '{print $1}')" != "$(cat "$TMPDIR/geoip.dat.sha256sum" | awk '{print $1}')" ]; then
        echo -e "\e[1;31mgeoip.dat checksum error\e[0m"
        rm -rf "$TMPDIR"
        exit 1
    fi

    # geosite.dat
    echo -e "\e[1;32mDownloading "$mirror"https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geosite.dat\e[0m"
    curl --connect-timeout 5 -m 120 --ipv4 -kfSLo "$TMPDIR/geosite.dat" ""$mirror"https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geosite.dat"
    [ $? -ne 0 ] && rm -rf "$TMPDIR" && exit 1
    # checksum - geosite.dat
    echo -e "\e[1;32mDownloading "$mirror"https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geosite.dat.sha256sum\e[0m"
    curl --connect-timeout 5 -m 10 --ipv4 -kfSLo "$TMPDIR/geosite.dat.sha256sum" ""$mirror"https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geosite.dat.sha256sum"
    [ $? -ne 0 ] && rm -rf "$TMPDIR" && exit 1
    if [ "$(sha256sum "$TMPDIR/geosite.dat" | awk '{print $1}')" != "$(cat "$TMPDIR/geosite.dat.sha256sum" | awk '{print $1}')" ]; then
        echo -e "\e[1;31mgeosite.dat checksum error\e[0m"
        rm -rf "$TMPDIR"
        exit 1
    fi
    rm -rf "$TMPDIR"/*.sha256sum
    \cp -a "$TMPDIR"/* /usr/share/v2ray
    rm -rf "$TMPDIR"
)

restart_service() {
    /etc/init.d/mosdns restart
}

flush_cache() {
    curl -s 127.0.0.1:$(uci -q get mosdns.config.listen_port_api)/plugins/lazy_cache/flush || exit 1
}

v2dat_dump() {
    # env
    v2dat_dir=/usr/share/v2ray
    adblock=$(uci -q get mosdns.config.adblock)
    ad_source=$(uci -q get mosdns.config.ad_source)
    configfile=$(uci -q get mosdns.config.configfile)
    mkdir -p /var/mosdns
    rm -f /var/mosdns/geo*.txt
    if [ "$configfile" = "/etc/mosdns/config.yaml" ]; then
        # default config
        v2dat unpack geoip -o /var/mosdns -f cn $v2dat_dir/geoip.dat
        v2dat unpack geosite -o /var/mosdns -f cn -f apple -f 'geolocation-!cn' $v2dat_dir/geosite.dat
        [ "$adblock" -eq 1 ] && [ $(echo $ad_source | grep -c geosite.dat) -ge '1' ] && v2dat unpack geosite -o /var/mosdns -f category-ads-all $v2dat_dir/geosite.dat
    else
        # custom config
        v2dat unpack geoip -o /var/mosdns -f cn $v2dat_dir/geoip.dat
        v2dat unpack geosite -o /var/mosdns -f cn -f 'geolocation-!cn' $v2dat_dir/geosite.dat
        geoip_tags=$(uci -q get mosdns.config.geoip_tags)
        geosite_tags=$(uci -q get mosdns.config.geosite_tags)
        [ -n "$geoip_tags" ] && v2dat unpack geoip -o /var/mosdns $(echo $geoip_tags | sed -r 's/\S+/-f &/g') $v2dat_dir/geoip.dat
        [ -n "$geosite_tags" ] && v2dat unpack geosite -o /var/mosdns $(echo $geosite_tags | sed -r 's/\S+/-f &/g') $v2dat_dir/geosite.dat
    fi
}

cloudflare_ip() {
    uci -q get mosdns.config.cloudflare_ip
}

case $script_action in
    "dns")
        interface_dns
    ;;
    "adlist")
        get_adlist
    ;;
    "geodata")
        geodat_update && adlist_update && restart_service
    ;;
    "logfile")
        logfile_path
    ;;
    "adlist_update")
        adlist_update && [ "$has_update" -eq 1 ] && restart_service
    ;;
    "flush")
        flush_cache
    ;;
    "v2dat_dump")
        v2dat_dump
    ;;
    "cloudflare")
        cloudflare_ip
    ;;
    "version")
        mosdns version
    ;;
    *)
        exit 0
    ;;
esac
