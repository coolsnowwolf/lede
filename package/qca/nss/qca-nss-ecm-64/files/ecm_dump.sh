#!/bin/sh
#
# Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

ECM_MODULE=${1:-ecm_state}
MOUNT_ROOT=/dev/ecm

#
# usage: ecm_dump.sh [module=ecm_db]
#
# with no parameters, ecm_dump.sh will attempt to mount the
# ecm_db state file and cat its contents.
#
# example with a parameter: ecm_dump.sh ecm_classifier_default
#
# this will cause ecm_dump to attempt to find and mount the state
# file for the ecm_classifier_default module, and if successful
# cat the contents.
#

# this is one of the state files, which happens to be the
# last module started in ecm
ECM_STATE=/sys/kernel/debug/ecm/ecm_state/state_dev_major

# tests to see if ECM is up and ready to receive commands.
# returns 0 if ECM is fully up and ready, else 1
ecm_is_ready() {
	if [ ! -e "${ECM_STATE}" ]
	then
		return 1
	fi
	return 0
}

#
# module_state_mount(module_name)
#      Mounts the state file of the module, if supported
#
module_state_mount() {
	local module_name=$1
	local mount_dir=$2
	local state_file="/sys/kernel/debug/ecm/${module_name}/state_dev_major"

	if [ -e "${mount_dir}/${module_name}" ]
	then
		# already mounted
		return 0
	fi

	#echo "Mount state file for $module_name ..."
	if [ ! -e "$state_file" ]
	then
		#echo "... $module_name does not support state"
		return 1
	fi

	local major="`cat $state_file`"
	#echo "... Mounting state $state_file with major: $major"
	mknod "${mount_dir}/${module_name}" c $major 0
}

#
# main
#
ecm_is_ready || {
	#echo "ECM is not running"
	exit 1
}

# all state files are mounted under MOUNT_ROOT, so make sure it exists
mkdir -p ${MOUNT_ROOT}

#
# attempt to mount state files for the requested module and cat it
# if the mount succeeded
#
module_state_mount ${ECM_MODULE} ${MOUNT_ROOT} && {
	cat ${MOUNT_ROOT}/${ECM_MODULE}
	exit 0
}

exit 2
