#!/bin/sh
#
# Copyright (C) 2019 p.t. <peter-tank@github.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

EXTRA_COMMANDS="update cache_file"

PROG=/usr/sbin/dnscrypt-proxy
CONFIG_DIR=/var/etc

LUCI_STATUS=/var/run/luci-reload-status
LOG_FILE="/var/etc/dnscrypt"
CACHE_DIR="/usr/share/dnscrypt-proxy"

#set -x

. $IPKG_INSTROOT/lib/functions.sh

multivalue_parse() {
    local param_values=$(echo $1 | sed -e 's/[, ]/\n/g' -e 's/\n\n/\n/g')

    [[ x == x${param_values} ]] && echo -n "[]" && return 1
    local p
    p=$(for p in ${param_values}; do echo -n "'${p}'"; done)
    echo -n "["
    echo -n $p | sed -e "s/''/', '/g"
    echo -n "]"
}

cache_file() {
    local tdl=$1
    local key=$2
    local root=$3
    local mode=${4:-0}
    [ -n "$root" ] && tdl=$tdl$'\n'$tdl.minisig
    for cdl in $tdl; do
    echo "INFO: details[$cdl]" > $LUCI_STATUS && sleep 3
    local md5=$(echo $cdl | md5sum | cut -d' ' -f1)
    local fn="$CACHE_DIR/${md5}_${cdl##*/}"
    local tmpf="/tmp/dnscrypt_${cdl##*/}.dl"
    local line
    [ ! -f "$fn" ] && {
        wget -q --no-check-certificate -t 3 -T 30 -O "$tmpf" "$cdl"
        [ $? -eq 0 ] || return
        line=$(cat "$tmpf" | wc -l)
        [ $line -gt 1 ] || return
        cat /dev/null > "$fn"
        [ -z "$key" -o $mode -eq 11 ] && echo "##source[#$line]: $cdl" >> "$fn"
        cat "$tmpf" >> "$fn"
    }
    [ $mode -ge 10 ] && cat "$fn" > $root/${cdl##*/}
    done
    echo "${fn%%.minisig}"
}

#update <resolvers_filename_in_configure>
update() {
    local cfg="${1}"
    local siteroot="$(uci_get uhttpd main home ""/www"")"

    [ -z "${cfg}" ] && exit 1
    NO_CALLBACK=1 config_load "${cfg}"
    local section="global" urls minisign_key cache_file details_json cache_mode selfsign

    config_get      urls             $section 'urls'             ''
    [ $? -ne 0 ] && echo "ERROR: Wrong configure..." > $LUCI_STATUS && sleep 3 && exit 1
    [ -z "${urls}" ] && echo "SKIPED: None resolvers URLs..." > $LUCI_STATUS && sleep 3 && exit 2
    . "$IPKG_INSTROOT/usr/share/libubox/jshn.sh"

    config_get      minisign_key     $section 'minisign_key'     ''
    config_get      cache_file       $section 'cache_file'       "${urls##*/}"
    config_get      details_json     $section 'details_json'     "${cache_file%%.md}.json"
    config_get_bool cache_mode       $section 'cache_mode'       '1'
    config_get_bool selfsign         $section 'selfsign'         '0'

    json_set_namespace "urls" "pre"
    json_init
    json_load "{'urls':$(multivalue_parse $urls)}"
    json_for_each_item cache_file 'urls' "$siteroot" "$cache_mode$selfsign"
    json_cleanup
    json_set_namespace "pre"

    local url="$details_json" stype="dnscrypt" file
    [ -z "${url}" ] && return 2

    if [ x"${url:0:4}" == x"http" ]; then file=$(cache_file "${url}" $cache_mode$selfsign);
    elif [ x"${url:0:1}" == x"/" ]; then file="${url}";
    else file="$CACHE_DIR/${url}";
    fi
    [ x"$file" == x -o ! -f "${file}" ] && { echo "ERROR: JSON file not exist!" > $LUCI_STATUS; sleep 3; return 3; }

    json_set_namespace "resolvers" "pre"
    json_init
    echo "INFO: JSON parsing [$file]..." > $LUCI_STATUS
    json_load "{'resolvers':$(cat $file)}"

    local type resolvers resolver keys key val
    local sname name addrs ports proto stamp country description dnssec ipv6 location nofilter nolog
    json_get_type type "resolvers"
    [ x"$type" == "xarray" ] || { echo "ERROR: JSON parse error!" > $LUCI_STATUS; sleep 3; return 4; }

    json_get_multivalue() {
        local _v_dst="$1"
        local _v_keys _v_val _select=
        local _json_no_warning=1 _sep=","

        unset "$_v_dst"
        [ -n "$2" ] && {
            json_select "$2" || return 1
            _select=1
        }

        json_get_keys _v_keys
        set -- $_v_keys
        while [ "$#" -gt 0 ]; do
            json_get_var _v_val "$1"
            __jshn_raw_append "$_v_dst" "$_v_val" "$_sep"
            shift
        done
        [ -n "$_select" ] && json_select ..

        return 0
    }
    json_select "resolvers"
        json_get_keys resolvers
        local icount=${resolvers##* }
        for resolver in $resolvers; do
           json_select "$resolver"
               json_get_vars name proto stamp country description dnssec ipv6 nofilter nolog
               json_get_multivalue addrs "addrs"
               json_get_multivalue ports "ports"
               json_get_multivalue location "location"
               sname=$(echo "$name" | sed 's/[^a-zA-Z0-9_]/_/g')
#               uci_add "$cfg" "$stype" "$sname"
#               for option in name proto addrs ports stamp location country description dnssec ipv6 nofilter nolog; do eval "uci_set \"$cfg\" \"$sname\" \"$option\" \"\$(echo \$$option | sed -e \"s/\'/,/g\")\""; done
               uci batch <<-EOB
set "$cfg.$sname"="$stype"
set "$cfg.$sname.name"="$name"
set "$cfg.$sname.proto"="$proto"
set "$cfg.$sname.addrs"="$addrs"
set "$cfg.$sname.ports"="$ports"
set "$cfg.$sname.stamp"="$stamp"
set "$cfg.$sname.location"="$location"
set "$cfg.$sname.country"="${country:-Unkown}"
set "$cfg.$sname.description"="$description"
set "$cfg.$sname.dnssec"="$dnssec"
set "$cfg.$sname.ipv6"="$ipv6"
set "$cfg.$sname.nofilter"="$nofilter"
set "$cfg.$sname.nolog"="$nolog"
EOB
               [ "${resolver:${#resolver}-1:1}" -eq 9 ] && uci_commit "$cfg" && echo "INFO: resolver[$resolver/${icount}]: $name {${proto}://$addrs|$ports}" > $LUCI_STATUS
           json_select ..
        done
    json_select ..
    uci_commit "$cfg" && echo "DONE: resolver[$resolver/${icount}]: $name {${proto}://$addrs|$ports}" > $LUCI_STATUS && sleep 3
    json_cleanup
    json_set_namespace "pre"
    return 0
}

echo "INFO: update resolvers[$1]" > $LUCI_STATUS && sleep 3
update $1

