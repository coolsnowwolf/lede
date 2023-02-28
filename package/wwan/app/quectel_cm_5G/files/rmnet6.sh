#!/bin/sh
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.

. /lib/functions.sh
. /lib/functions/network.sh
. ../netifd-proto.sh
init_proto "$@"

proto_rmnet6_setup() {
	local cfg="$1"
	local iface="$2"

	logger "rmnet6 started"
	#Call rmnet management script below!!
	/lib/netifd/rmnet6.script renew $cfg $iface
	logger "rmnet6 updated"
}

proto_rmnet6_teardown() {
	local cfg="$1"
	#Tear down rmnet manager script here.*/
}

proto_rmnet6_init_config() {
	#ddno_device=1
	available=1
}

add_protocol rmnet6
