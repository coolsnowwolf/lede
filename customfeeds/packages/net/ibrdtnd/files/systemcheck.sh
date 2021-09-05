#!/bin/sh
#
#

check_mounted() {
	DIR=$1
	while [ "$DIR" != "/" ]; do
		if [ -n "`mount | grep "$DIR"`" ]; then
			return 0
		fi

		DIR=`dirname $DIR`
	done
	return 1
}

check_writable() {
	CHECKFILE="$1/.container-lock"
	/bin/touch $CHECKFILE

	if [ $? -gt 0 ]; then
		return 1;
	fi

	/bin/echo "0123456789" >> $CHECKFILE

	if [ $? -gt 0 ]; then
		return 2;
	fi

	/bin/rm $CHECKFILE

	if [ $? -gt 0 ]; then
		return 3;
	fi
}

check_mountdev() {
	# get wait_mount option
	WAIT_MOUNT_DEV=`uci -q get ibrdtn.safemode.wait_mount`
	
	if [ $? -ne 0 ]; then
		return 0
	fi
	
	DATA=`mount | grep ${WAIT_MOUNT_DEV}`

	if [ -n "${DATA}" ]; then
		return 0
	fi
	
	return 1
}

# check the storage device
check_mountdev
RET=$?

if [ ${RET} -ne 0 ]; then
	WAIT_SECONDS=60
	/usr/bin/logger -t "systemcheck.sh" -p 2 "disk storage not ready, wait max. ${WAIT_SECONDS} seconds until it is mounted"
	while [ ${RET} -ne 0 ] && [ ${WAIT_SECONDS} -ne 0 ]; do
		sleep 1
		let WAIT_SECONDS=WAIT_SECONDS-1
		check_mountdev
		RET=$?
	done
fi

if [ ${RET} -ne 0 ]; then
	# failed, storage not mounted
	exit 1
fi

# get the path for the container
CONTAINER=`uci -q get ibrdtn.storage.container`

if [ -z "$CONTAINER" ]; then
	exit 0
fi

CONTAINER_PATH=`dirname $CONTAINER`

if [ -n "$CONTAINER_PATH" ]; then
	# check if the container is on a mounted device
	check_mounted $CONTAINER_PATH

	if [ $? -gt 0 ]; then
		# failed
		exit 1
	fi

	# check if the device is writable
	check_writable $CONTAINER_PATH

	if [ $? -gt 0 ]; then
		# failed
		exit 1
	fi
fi

