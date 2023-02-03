#!/bin/sh
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.

. /lib/functions.sh
. /lib/functions/network.sh
. ../netifd-proto.sh
init_proto "$@"

proto_rmnet_setup() {
	local cfg="$1"
	local iface="$2"

	logger "rmnet started"
	#Call rmnet management script below!!
	logger "rmnet updated ${cfg} ${iface}"
	/lib/netifd/rmnet.script renew $cfg $iface
}

proto_rmnet_teardown() {
	local cfg="$1"
	#Tear down rmnet manager script here.*/
}

proto_rmnet_init_config() {
	#ddno_device=1
	available=1
}

add_protocol rmnet
