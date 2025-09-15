#!/bin/sh

. /usr/share/libubox/jshn.sh

addenv="$( env | while read line; do echo "json_add_string \"\" \"$line\";"; done )"
json_init
json_add_array env
json_add_string "" "ACTION=$1"
eval "$addenv"
json_close_array env

ubus call hotplug.ntp call "$(json_dump)"
