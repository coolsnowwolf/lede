#
# Copyright (C) 2018 Weijie Gao <hackpascal@gmail.com>
#
# Helper function to extract mac addresses from mtd part for Phicomm K2T
#

. /lib/functions.sh
. /lib/functions/system.sh
. /usr/share/libubox/jshn.sh

k2t_config_load() {
	local mtd_blk=$(find_mtd_part config)

	if [ -z "$mtd_blk" ]; then
		echo "k2t_config_load: no mtd part named config" >&2
		exit 1
	fi

	local json_size=$(dd if=$mtd_blk bs=1 count=8 2>/dev/null)

	json_size="0x$json_size"
	json_size=$((json_size))

	if [ "$?" -ne 0 ]; then
		echo "k2t_config_load: invalid json data size" >&2
		exit 2
	fi

	if [ "$json_size" -eq 0 ]; then
		echo "k2t_config_load: empty json data" >&2
		exit 3
	fi

	local json_data=$(dd if=$mtd_blk bs=1 skip=8 count=$json_size 2>/dev/null)

	json_load "$json_data"
}

k2t_get_mac() {
	local old_ns

	json_set_namespace "k2t" old_ns

	if k2t_config_load; then
		json_select "this_dev_info"
		json_get_var val "$1"
		json_select ..
	fi

	json_set_namespace old_ns

	echo $val
}

