#!/bin/sh
#

CONTAINER=`/sbin/uci -q get ibrdtn.storage.container`
CPATH=`/sbin/uci -q get ibrdtn.storage.path`

check_var() {
	if [ -z "$1" ]; then
		echo "$2"
		exit 1
	fi
}

check_path() {
	if [ ! -d "$1" ]; then
		echo "$2"
		return 1
	fi
}

check_file() {
	if [ ! -f "$1" ]; then
		echo "$2"
		exit 1
	fi
}

container_mount() {
	CONTAINER=`/sbin/uci -q get ibrdtn.storage.container`
	CPATH=`/sbin/uci -q get ibrdtn.storage.path`

	if [ -z "`mount | grep ' on $CPATH '`" ]; then
		# try to mount the container
		/bin/mount -o loop $CONTAINER $CPATH

		return $?
	fi

	return 0
}

container_reinitialize() {
	SIZE=`/sbin/uci get -q ibrdtn.storage.container_size`
	CONTAINER=`/sbin/uci -q get ibrdtn.storage.container`

	# try to rebuild the container
	if [ -n "$SIZE" ]; then
		/bin/rm -f $CONTAINER
		/usr/share/ibrdtn/mkcontainer.sh $CONTAINER $SIZE

		if [ $? -eq 0 ]; then
			container_mount
			return $?
		fi

		return 1
	fi

	return 1
}

check_var $CONTAINER "Storage container not set in uci.\nuse: uci set ibrdtn.storage.container=<container-file>"
check_var $CPATH "Storage container mount path not set in uci.\nuse: uci set ibrdtn.storage.path=<mount-path>"

check_path $CPATH "Storage container mount path does not exist."
if [ $? -gt 0 ]; then
	/bin/mkdir -p $CPATH

	if [ $? -gt 0 ]; then
		echo "can not create container mount path."
		exit 1
	fi
fi

if [ "$1" == "-u" ]; then
	/bin/umount $CPATH
	exit 0
fi

if [ -n "`/bin/mount | grep $CPATH`" ]; then
	echo "Container already mounted"
	exit 0
fi

if [ ! -f "$CONTAINER" ]; then
	echo "Storage container file $CONTAINER does not exist."
	container_reinitialize
	exit $?
fi

# try to mount the container
container_mount

if [ $? -gt 0 ]; then
	echo -n "can not mount container file. checking... "
	/usr/sbin/e2fsck -p $CONTAINER

	if [ $? -gt 0 ]; then
		echo " error"
		echo "Container file $CONTAINER broken. Try to reinitialize the container."
		container_reinitialize

		if [ $? -eq 0 ]; then
			echo "container ready!"
			exit 0
		else
			exit 1
		fi
	fi
	echo "done"

	container_mount

	if [ $? -gt 0 ]; then
		echo "mount failed!"
		exit 1
	fi
fi

echo "container ready!"
exit 0

