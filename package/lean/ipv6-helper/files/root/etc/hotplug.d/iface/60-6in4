#!/bin/sh
. /lib/functions.sh
if [ "$ACTION" != "ifup" ]; then
    exit
fi
config_load network
config_get tunnelid $INTERFACE tunnelid
config_get username $INTERFACE username
config_get password $INTERFACE password
if [ "$tunnelid" != "" ]; then
    wget -O - https://$username:$password@ipv4.tunnelbroker.net/nic/update?hostname=$tunnelid --no-check-certificate
fi
